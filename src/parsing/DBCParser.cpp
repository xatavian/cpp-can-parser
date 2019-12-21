#include "DBCParser.h"
#include "CANDatabaseException.h"
#include <string>
#include <iostream>
#include <algorithm>
#include "ParsingUtils.h"

using namespace DBCParser;

std::shared_ptr<CANSignal> parseSignal(Tokenizer& tokenizer);
std::shared_ptr<CANFrame>  parseFrame(Tokenizer& tokenizer);
std::set<std::string>      parseECUs(Tokenizer& tokenizer);
void                       addBADirective(Tokenizer& tokenizer,
                                          CANDatabase& db);
void                       addComment(Tokenizer& tokenizer, CANDatabase& db);
void                       parseSignalChoices(Tokenizer& tokenizer,
                                           CANDatabase& db);

void addComment(Tokenizer& tokenizer, CANDatabase& db) {
  Token commentType;
  Token commentValue;
  Token targetFrame;
  Token targetSignal;

  assertToken(tokenizer, "CM_");
  commentType = checkTokenType(tokenizer, Token::Identifier);

  auto wWrongFrameId = [](const std::string& fId, unsigned long long line) {
    warning("Frame " + fId + " does not exist", line);
  };
  if(commentType.image() == "SG_") {
    targetFrame = checkTokenType(tokenizer, Token::Number);
    targetSignal = checkTokenType(tokenizer, Token::Identifier);
    commentValue = checkTokenType(tokenizer, Token::Literal);
    skipIf(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.hasFrame(frame_id)) {
      wWrongFrameId(targetFrame.image(), tokenizer.lineCount());
      return;
    }
    else if(!db.getFrameById(frame_id).lock()->hasSignal(targetSignal.image())) {
      warning("Frame " + targetFrame.image() +
              "has no signal \"" + targetSignal.image() + "\"",
            tokenizer.lineCount());
      return;
    }
    db.getFrameById(frame_id).lock()
      ->getSignalByName(targetSignal.image()).lock()
      ->setComment(commentValue.image());
  }
  else if(commentType.image() == "BO_") {
    targetFrame = checkTokenType(tokenizer, Token::Number);
    commentValue = checkTokenType(tokenizer, Token::Literal);
    skipIf(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.hasFrame(frame_id)) {
      wWrongFrameId(targetFrame.image(), tokenizer.lineCount());
      return;
    }

    db.getFrameById(frame_id).lock()
      ->setComment(commentValue.image());
  }
  else {
    warning("Unsupported comment operation \"" +
            commentType.image() + "\"",
            tokenizer.lineCount());
    tokenizer.skipUntil(";");
  }
}

void addBADirective(Tokenizer& tokenizer, CANDatabase& db) {
  Token infoType;
  assertToken(tokenizer, "BA_");

  infoType = checkTokenType(tokenizer, Token::Literal);
  if(infoType.image() == "GenMsgCycleTime") {
    skipIf(tokenizer, "BO_");
    Token frameId = checkTokenType(tokenizer, Token::Number);
    Token period = checkTokenType(tokenizer, Token::Number);
    skipIf(tokenizer, ";");

    if(period.image()[0] == '-') {
      warning("cannot set negative period",
              tokenizer.lineCount());
      return;
    }
    else if(frameId.image()[0] == '-') {
      warning("invalid frame id",
              tokenizer.lineCount());
      return;
    }

    unsigned int iFrameId = std::stoul(frameId.image());
    unsigned int iPeriod = std::stoul(period.image());

    auto frame = db.getFrameById(iFrameId);
    if(frame.expired()) {
      std::cout << "WARNING: frame " << iPeriod << " does not exist "
                << "at line " << tokenizer.lineCount()
                << std::endl;
      return;
    }
    frame.lock()->setPeriod(iPeriod);
  }
  else {
    std::cout << "WARNING: Unrecognized BA_ command " << infoType.image()
              << " at line " << tokenizer.lineCount()
              << std::endl;
    tokenizer.skipUntil(";");
  }
}

CANDatabase DBCParser::fromTokenizer(const std::string& name, Tokenizer& tokenizer) {
  std::cout << "Parsing: " << std::endl;
  Token currentToken = tokenizer.getNextToken();

  CANDatabase result(name);

  while(currentToken.type() != Token::Eof) {
    // std::cout << currentToken.image() << std::endl;
    if(currentToken.image() == "VERSION") {
      currentToken = checkTokenType(tokenizer, Token::Literal);
      std::cout << "DBC version: " << currentToken.image() << std::endl;
    }
    else if(currentToken.image() == "BU_") {
      std::set<std::string> ecus = parseECUs(tokenizer);
      std::cout << "The following ECUs have been defined:" << std::endl;
      for(const auto& ecu : ecus) {
        std::cout << ecu << ", ";
      }
      std::cout << std::endl;
    }
    else if(currentToken.image() == "BO_") {
      std::shared_ptr<CANFrame> frame = parseFrame(tokenizer);
      result.addFrame(frame);
    }
    else if(currentToken.image() == "SG_") {
      parseSignal(tokenizer);
      std::cout << "Identified signal outside frame -> WARNING !!!"
                << std::endl;
    }
    else if(currentToken.image() == "CM_") {
      addComment(tokenizer, result);
      // TODO: Handle comments
    }
    else if(currentToken.image() == "BA_") {
      addBADirective(tokenizer, result);
    }
    else if(currentToken.image() == "VAL_") {
      parseSignalChoices(tokenizer, result);
    }
    currentToken = tokenizer.getNextToken();
  }

  return result;
}

std::shared_ptr<CANSignal> parseSignal(Tokenizer& tokenizer) {
  Token signalName, startBit, length,
        endianess, signedness, scale,
        offset, min, max, unit, targetECU;

  assertToken(tokenizer, "SG_");

  signalName = checkTokenType(tokenizer, Token::Identifier);
  skipIf(tokenizer, ":");
  startBit = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, "|");
  length = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, "@");
  endianess = checkTokenType(tokenizer, Token::Number);
  signedness = checkTokenType(tokenizer, Token::Sign);
  skipIf(tokenizer, "(");
  scale = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, ",");
  offset = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, ")");
  skipIf(tokenizer, "[");
  min = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, "|");
  max = checkTokenType(tokenizer, Token::Number);
  skipIf(tokenizer, "]");
  unit = checkTokenType(tokenizer, Token::Literal);
  targetECU = checkTokenType(tokenizer, Token::Identifier); // Ignored for now

  return std::make_shared<CANSignal>(
    signalName.image(),
    std::stoul(startBit.image()),
    std::stoul(length.image()),
    std::stof(scale.image()),
    std::stof(offset.image()),
    signedness.image() == "-" ? CANSignal::Signed : CANSignal::Unsigned,
    endianess.image() == "0" ? CANSignal::BigEndian : CANSignal::LittleEndian,
    CANSignal::Range::fromString(min.image(), max.image())
  );
}

std::shared_ptr<CANFrame> parseFrame(Tokenizer& tokenizer) {
  Token name;
  Token id;
  Token dlc;
  Token ecu;

  assertToken(tokenizer, "BO_");

  id = checkTokenType(tokenizer, Token::Number);
  name = checkTokenType(tokenizer, Token::Identifier);

  skipIf(tokenizer, ":");

  dlc = checkTokenType(tokenizer, Token::Number);
  ecu = checkTokenType(tokenizer, Token::Identifier);

  std::shared_ptr<CANFrame> result = std::make_shared<CANFrame>(
    name.image(),
    std::stoul(id.image()),
    std::stoul(dlc.image())
  );

  Token currentToken = tokenizer.getNextToken();

  while(currentToken.image() == "SG_") {
    result->addSignal(parseSignal(tokenizer));
    currentToken = tokenizer.getNextToken();
  }

  if(currentToken.type() != Token::Eof)
    tokenizer.saveToken(currentToken);

  return result;
}

std::set<std::string> parseECUs(Tokenizer& tokenizer) {
  std::set<std::string> result;

  assertToken(tokenizer, "BU_");
  skipIf(tokenizer, ":");
  unsigned long long currentLine = tokenizer.lineCount();

  Token currentToken = checkTokenType(tokenizer, Token::Identifier);

  // Looking for all the identifiers on the same line
  while(currentToken.type() != Token::Eof &&
        currentLine == tokenizer.lineCount()) {
    result.insert(currentToken.image());
    currentToken = checkTokenType(tokenizer, Token::Identifier);
  }

  if(currentToken.type() != Token::Eof)
    tokenizer.saveToken(currentToken);

  return result;
}

void parseSignalChoices(Tokenizer& tokenizer, CANDatabase& db) {
  Token targetFrame;
  Token targetSignal;
  Token currentVal;
  Token currentLabel;

  std::map<unsigned int, std::string> targetChoices;

  assertToken(tokenizer, "VAL_");
  targetFrame = checkTokenType(tokenizer, Token::Number);
  targetSignal = checkTokenType(tokenizer, Token::Identifier);

  Token currentToken = tokenizer.getNextToken();
  while(currentToken.image() != ";" &&
        currentToken.type() != Token::Eof) {
    currentVal = checkCurrentTokenType(currentToken, Token::Number,
                                       tokenizer.lineCount());
    currentLabel = checkTokenType(tokenizer, Token::Literal);

    targetChoices.insert(
      std::make_pair(
        currentVal.toUInt(),
        currentLabel.image()
      )
    );

    currentToken = tokenizer.getNextToken();
  }
  checkCurrentTokenType(currentToken, ";", tokenizer.lineCount());

  unsigned long long frame_id = targetFrame.toUInt();
  if(!db.hasFrame(frame_id) ||
     !db.getFrameById(frame_id).lock()->hasSignal(targetSignal.image())) {
    warning("Cannot assign enum to signal \"" + targetFrame.image() + "/" +
            targetSignal.image() + "\"",
            tokenizer.lineCount());
    return;
  }

  db.getFrameById(frame_id).lock()
    ->getSignalByName(targetSignal.image()).lock()
    ->setChoices(targetChoices);
}
