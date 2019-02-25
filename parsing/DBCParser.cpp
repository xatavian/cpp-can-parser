#include "parsing/DBCParser.h"
#include "models/CANDatabaseException.h"
#include <string>
#include <iostream>
#include <algorithm>
using namespace DBCParser;

void skipIf(Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getNextToken().image() != token) {
    throw CANDatabaseException(
      "Syntax error: expected " +
      token +
      " but got " +
      tokenizer.getCurrentToken().image() +
      " at line " + std::to_string(tokenizer.lineCount())
    );
  }
}

void assertToken(const Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getCurrentToken().image() != token) {
    throw CANDatabaseException(
      "Syntax error: expected " +
      token +
      " but got " +
      tokenizer.getCurrentToken().image() +
      " at line " + std::to_string(tokenizer.lineCount())
    );
  }
}

Token checkTokenType(Tokenizer& tokenizer, Token::Type targetType) {
  Token result = tokenizer.getNextToken();
  if(result.type() != targetType) {
    throw CANDatabaseException(
      "Syntax error: unexpected \"" + result.image() +
      "\" at line " + std::to_string(tokenizer.lineCount())
    );
  }

  return result;
}

void addAdditionalInformation(Tokenizer& tokenizer, CANDatabase& db) {
  Token infoType;
  assertToken(tokenizer, "BA_");

  infoType = checkTokenType(tokenizer, Token::Literal);
  if(infoType.image() == "\"GenMsgCycleTime\"") {
    skipIf(tokenizer, "BO_");
    Token frameId = checkTokenType(tokenizer, Token::Number);
    Token period = checkTokenType(tokenizer, Token::Number);
    skipIf(tokenizer, ";");

    if(period.image()[0] == '-') {
      std::cout << "WARNING: cannot set negative period at line "
                << tokenizer.lineCount() << std::endl;
      return;
    }
    else if(frameId.image()[0] == '-') {
      std::cout << "WARNING: cannot set period for a frame with a "
                << "negative frame id at line " << tokenizer.lineCount()
                << std::endl;
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
      tokenizer.skipLine();
      // TODO: Handle comments
    }
    else if(currentToken.image() == "BA_") {
      addAdditionalInformation(tokenizer, result);
    }

    currentToken = tokenizer.getNextToken();
  }

  return result;
}

std::shared_ptr<CANSignal> DBCParser::parseSignal(Tokenizer& tokenizer) {
  Token signalName;
  Token startBit;
  Token length;
  Token endianess;
  Token signedness;
  Token scale;
  Token offset;
  Token min;
  Token max;
  Token unit;
  Token targetECU;

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

std::shared_ptr<CANFrame> DBCParser::parseFrame(Tokenizer& tokenizer) {
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

std::set<std::string> DBCParser::parseECUs(Tokenizer& tokenizer) {
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
