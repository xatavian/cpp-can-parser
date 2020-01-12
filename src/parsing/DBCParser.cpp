#include "DBCParser.h"
#include "CANDatabaseException.h"
#include <string>
#include <iostream>
#include <algorithm>
#include "ParsingUtils.h"

using namespace DBCParser;

CANSignal                  parseSignal(Tokenizer& tokenizer);
CANFrame                   parseFrame(Tokenizer& tokenizer);
std::set<std::string>      parseECUs(Tokenizer& tokenizer);
void                       parseNewSymbols(Tokenizer& tokenizer);
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
  
  // Handle global comment
  Token currentToken = tokenizer.getNextToken();
  if (currentToken.type() == Token::Literal) {
    skipIf(tokenizer, ";");
    return;
  }

  commentType = checkCurrentTokenType(currentToken, Token::Identifier, tokenizer.lineCount());

  auto wWrongFrameId = [](const std::string& fId, unsigned long long line) {
    warning("Frame " + fId + " does not exist", line);
  };

  if(commentType.image() == "SG_") {
    targetFrame = checkTokenType(tokenizer, Token::Number);
    targetSignal = checkTokenType(tokenizer, Token::Identifier);
    commentValue = checkTokenType(tokenizer, Token::Literal);
    skipIf(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.contains(frame_id)) {
      wWrongFrameId(targetFrame.image(), tokenizer.lineCount());
      return;
    }
    else if(!db.at(frame_id).contains(targetSignal.image())) {
      warning("Frame " + targetFrame.image() +
              "has no signal \"" + targetSignal.image() + "\"",
            tokenizer.lineCount());
      return;
    }
    db.at(frame_id)
      .at(targetSignal.image())
      .setComment(commentValue.image());
  }
  else if(commentType.image() == "BO_") {
    targetFrame = checkTokenType(tokenizer, Token::Number);
    commentValue = checkTokenType(tokenizer, Token::Literal);
    skipIf(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.contains(frame_id)) {
      wWrongFrameId(targetFrame.image(), tokenizer.lineCount());
      return;
    }

    db.at(frame_id).setComment(commentValue.image());
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
  if(infoType.image() == "GenMsgCycleTime" || infoType.image() == "CycleTime") {
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

    unsigned int iFrameId = frameId.toUInt();
    unsigned int iPeriod = period.toUInt();

    try {
      db.at(iFrameId).setPeriod(iPeriod);
    }
    catch (const std::out_of_range& e) {
      std::string tempStr = std::to_string(iFrameId) + " does not exist at line " + std::to_string(tokenizer.lineCount());
      throw CANDatabaseException(tempStr);
    }
  }
  else {
    std::cout << "WARNING: Unrecognized BA_ command " << infoType.image()
              << " at line " << tokenizer.lineCount()
              << std::endl;
    tokenizer.skipUntil(";");
  }
}

CANDatabase DBCParser::fromTokenizer(Tokenizer& tokenizer) {
  return fromTokenizer("", tokenizer);
}

void parseNewSymbols(Tokenizer& tokenizer) {
  static std::set<std::string> ns_choices = {
    "CM_", "BA_DEF_", "BA_", "VAL_", "CAT_DEF_", "CAT_", "FILTER", "BA_DEF_DEF_",
    "EV_DATA_", "ENVVAR_DATA", "SGTYPE_", "SGTYPE_VAL_", "BA_DEF_SGTYPE_", "BA_SGTYPE_",
    "SIG_TYPE_DEF_"
  };
  
  assertToken(tokenizer, "NS_");
  skipIf(tokenizer, ":");
  
  Token token = tokenizer.getNextToken();
  while (ns_choices.find(token.image()) != ns_choices.end()) {
    token = tokenizer.getNextToken();
  }
  tokenizer.saveToken(token);
}

CANDatabase DBCParser::fromTokenizer(const std::string& name, Tokenizer& tokenizer) {
  std::cout << "Parsing: " << std::endl;
  Token currentToken = tokenizer.getNextToken();

  CANDatabase result(name);

  while(currentToken.type() != Token::Eof) {
    // std::cout << currentToken.image() << std::endl;
    if (currentToken.image() == "VERSION") {
      currentToken = checkTokenType(tokenizer, Token::Literal);
      std::cout << "DBC version: " << currentToken.image() << std::endl;
    }
    else if (currentToken.image() == "BU_") {
      std::set<std::string> ecus = parseECUs(tokenizer);
      std::cout << "The following ECUs have been defined:" << std::endl;
      for (const auto& ecu : ecus) {
        std::cout << ecu << ", ";
      }
      std::cout << std::endl;
    }
    else if (currentToken.image() == "BO_") {
      result.addFrame(parseFrame(tokenizer));
    }
    else if (currentToken.image() == "SG_") {
      parseSignal(tokenizer);
      std::cout << "Identified signal outside frame -> WARNING !!! (line "
        << tokenizer.lineCount() << ")" << std::endl;
    }
    else if (currentToken.image() == "CM_") {
      addComment(tokenizer, result);
      // TODO: Handle comments
    }
    else if (currentToken.image() == "BA_") {
      addBADirective(tokenizer, result);
    }
    else if (currentToken.image() == "VAL_") {
      parseSignalChoices(tokenizer, result);
    }
    else if (currentToken.image() == "NS_") {
      parseNewSymbols(tokenizer);
    }
    else if (currentToken.image() == "BS_") {
      skipIf(tokenizer, ":");

      currentToken = tokenizer.getNextToken();
      if (currentToken.type() != Token::Number)
        continue;

      Token baudrate = checkCurrentTokenType(currentToken, Token::Number, tokenizer.lineCount());
      skipIf(tokenizer, ":");
      Token btr1 = checkTokenType(tokenizer, Token::Number);
      skipIf(tokenizer, ",");
      Token btr2 = checkTokenType(tokenizer, Token::Number);

      // TODO: handle the statement
    }
    else {
      std::cerr << currentToken.image() << " is not a valid statement (yet). The statement is skipped." << std::endl;
      tokenizer.skipUntil(";");
    }
    currentToken = tokenizer.getNextToken();
  }

  return result;
}

CANSignal parseSignal(Tokenizer& tokenizer) {
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
  Token currentToken = tokenizer.getNextToken();
  while (currentToken.image() == ",") {
    targetECU = checkTokenType(tokenizer, Token::Identifier);
    currentToken = tokenizer.getNextToken();
  }

  if (currentToken.type() != Token::Eof)
    tokenizer.saveToken(currentToken);

  return CANSignal(
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

CANFrame parseFrame(Tokenizer& tokenizer) {
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

  CANFrame result(
    name.image(), id.toUInt(), dlc.toUInt());

  Token currentToken = tokenizer.getNextToken();

  while(currentToken.image() == "SG_") {
    result.addSignal(parseSignal(tokenizer));
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
  if(!db.contains(frame_id) ||
     !db.at(frame_id).contains(targetSignal.image())) {
    warning("Cannot assign enum to signal \"" + targetFrame.image() + "/" +
            targetSignal.image() + "\"",
            tokenizer.lineCount());
    return;
  }

  db.at(frame_id)
    .at(targetSignal.image())
    .setChoices(targetChoices);
}
