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

static std::string VERSION_TOKEN = "VERSION";
static std::string BIT_TIMING_TOKEN = "BS_";
static std::string NODE_DEF_TOKEN = "BU_";
static std::string MESSAGE_DEF_TOKEN = "BO_";
static std::string SIG_DEF_TOKEN = "SG_";
static std::string SIG_VAL_DEF_TOKEN = "VAL_";
static std::string ENV_VAR_TOKEN = "EV_";
static std::string COMMENT_TOKEN = "CM_";
static std::string ATTR_DEF_TOKEN = "BA_DEF_";
static std::string ATTR_DEF_DEFAULT_TOKEN = "BA_DEF_DEF_";
static std::string ATTR_VAL_TOKEN = "BA_";

// Duplicates but I don't think it demands so much memory
// anyway...
static std::set<std::string> SUPPORTED_DBC_TOKENS = {
  VERSION_TOKEN, BIT_TIMING_TOKEN, NODE_DEF_TOKEN, MESSAGE_DEF_TOKEN,
  SIG_DEF_TOKEN, SIG_VAL_DEF_TOKEN, ENV_VAR_TOKEN, COMMENT_TOKEN,
  ATTR_DEF_TOKEN, ATTR_DEF_DEFAULT_TOKEN, ATTR_VAL_TOKEN
};

static std::set<std::string> NS_TOKENS = {
  "CM_", "BA_DEF_", "BA_", "VAL_", "CAT_DEF_", "CAT_", "FILTER", "BA_DEF_DEF_",
  "EV_DATA_", "ENVVAR_DATA", "SGTYPE_", "SGTYPE_VAL_", "BA_DEF_SGTYPE_", "BA_SGTYPE_",
  "SIG_TYPE_DEF_"
};

static std::set<std::string> UNSUPPORTED_DBC_TOKENS = {
  "VAL_TABLE_", "BO_TX_BU_", "ENVVAR_DATA_",
  "SGTYPE_", "SIG_GROUP_"
}; 

  
void addComment(Tokenizer& tokenizer, CANDatabase& db) {
  Token commentType;
  Token commentValue;
  Token targetFrame;
  Token targetSignal;

  assert_current_token(tokenizer, COMMENT_TOKEN);
  
  // Handle global comment
  Token currentToken = tokenizer.getNextToken();
  if (currentToken == Token::StringLiteral) {
    assert_token(tokenizer, ";");
    return;
  }

  commentType = assert_current_token(tokenizer, Token::Identifier);

  auto wWrongFrameId = [](const std::string& fId, unsigned long long line) {
    warning("Frame " + fId + " does not exist", line);
  };

  if(commentType == SIG_DEF_TOKEN) {
    targetFrame = assert_token(tokenizer, Token::Number);
    targetSignal = assert_token(tokenizer, Token::Identifier);
    commentValue = assert_token(tokenizer, Token::StringLiteral);
    assert_token(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.contains(frame_id)) {
      wWrongFrameId(targetFrame.image, tokenizer.lineCount());
      return;
    }
    else if(!db.at(frame_id).contains(targetSignal.image)) {
      warning("Frame " + targetFrame.image +
              "has no signal \"" + targetSignal.image + "\"",
            tokenizer.lineCount());
      return;
    }
    db.at(frame_id)
      .at(targetSignal.image)
      .setComment(commentValue.image);
  }
  else if(commentType == MESSAGE_DEF_TOKEN) {
    targetFrame = assert_token(tokenizer, Token::Number);
    commentValue = assert_token(tokenizer, Token::StringLiteral);
    assert_token(tokenizer, ";");

    auto frame_id = targetFrame.toUInt();
    if(!db.contains(frame_id)) {
      wWrongFrameId(targetFrame.image, tokenizer.lineCount());
      return;
    }

    db.at(frame_id).setComment(commentValue.image);
  }
  else {
    warning("Unsupported comment operation \"" +
            commentType.image + "\"",
            tokenizer.lineCount());
    tokenizer.skipUntil(";");
  }
}

void addBADirective(Tokenizer& tokenizer, CANDatabase& db) {
  Token infoType;
  assert_current_token(tokenizer, "BA_");

  infoType = assert_token(tokenizer, Token::StringLiteral);
  if(infoType == "GenMsgCycleTime" || infoType == "CycleTime") {
    assert_token(tokenizer, "BO_");
    Token frameId = assert_token(tokenizer, Token::Number);
    Token period = assert_token(tokenizer, Token::Number);
    assert_token(tokenizer, ";");

    if(period == Token::NegativeNumber) {
      warning("cannot set negative period",
              tokenizer.lineCount());
      return;
    }
    else if(frameId == Token::NegativeNumber) {
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
    std::cout << "WARNING: Unrecognized BA_ command " << infoType.image
              << " at line " << tokenizer.lineCount()
              << std::endl;
    tokenizer.skipUntil(";");
  }
}

CANDatabase DBCParser::fromTokenizer(Tokenizer& tokenizer) {
  return fromTokenizer("", tokenizer);
}

void parseNewSymbols(Tokenizer& tokenizer) {
  assert_current_token(tokenizer, "NS_");
  assert_token(tokenizer, ":");
  
  Token token = tokenizer.getNextToken();
  while (NS_TOKENS.count(token.image) > 0) {
    token = tokenizer.getNextToken();
  }
  tokenizer.saveToken(token);
}

CANDatabase DBCParser::fromTokenizer(const std::string& name, Tokenizer& tokenizer) {
  std::cout << "Parsing: " << std::endl;

  CANDatabase result(name);

  while(!is_token(tokenizer, Token::Eof)) {
    // std::cout << currentToken.image << std::endl;
    if(is_current_token(tokenizer, "VERSION")) {
      /*Token candb_version = */ assert_token(tokenizer, Token::StringLiteral);
      // std::cout << "CANdb++ version: " << candb_version.image << std::endl;
    }
    else if(is_current_token(tokenizer, "NS_")) {
      parseNewSymbols(tokenizer);
    }
    else if (is_current_token(tokenizer, "BU_")) {
      std::set<std::string> ecus = parseECUs(tokenizer);
      std::cout << "The following ECUs have been defined:" << std::endl;
      for (const auto& ecu : ecus) {
        std::cout << ecu << ", ";
      }
      std::cout << std::endl;
    }
    else if (is_current_token(tokenizer, "BO_")) {
      result.addFrame(parseFrame(tokenizer));
    }
    else if (is_current_token(tokenizer, "SG_")) {
      parseSignal(tokenizer);
      std::cout << "Identified signal outside frame -> WARNING !!! (line "
        << tokenizer.lineCount() << ")" << std::endl;
    }
    else if (is_current_token(tokenizer, "CM_")) {
      addComment(tokenizer, result);
      // TODO: Handle comments
    }
    else if (is_current_token(tokenizer, "BA_")) {
      addBADirective(tokenizer, result);
    }
    else if (is_current_token(tokenizer, "VAL_")) {
      parseSignalChoices(tokenizer, result);
    }
    else if (is_current_token(tokenizer, "BS_")) {
      assert_token(tokenizer, ":");

      if (is_token(tokenizer, Token::Number))
        continue;

      Token baudrate = assert_current_token(tokenizer, Token::PositiveNumber);
      assert_token(tokenizer, ":");
      Token btr1 = assert_token(tokenizer, Token::PositiveNumber);
      assert_token(tokenizer, ",");
      Token btr2 = assert_token(tokenizer, Token::PositiveNumber);

      // TODO: handle the statement
    }
    else {
      Token currentToken = tokenizer.getCurrentToken();
      std::cerr << currentToken.image << " is not a valid statement (yet). The statement is skipped." << std::endl;
      tokenizer.skipUntil(";");
    }
  }

  return result;
}

CANSignal parseSignal(Tokenizer& tokenizer) {
  Token signalName, startBit, length,
        endianess, signedness, scale,
        offset, min, max, unit, targetECU;

  assert_current_token(tokenizer, "SG_");

  signalName = assert_token(tokenizer, Token::Identifier);
  assert_token(tokenizer, ":");
  startBit = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "|");
  length = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "@");
  endianess = assert_token(tokenizer, Token::Number);
  signedness = assert_token(tokenizer, Token::ArithmeticSign);
  assert_token(tokenizer, "(");
  scale = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, ",");
  offset = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, ")");
  assert_token(tokenizer, "[");
  min = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "|");
  max = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "]");
  unit = assert_token(tokenizer, Token::StringLiteral);
   
  targetECU = assert_token(tokenizer, Token::Identifier); // Ignored for now
  Token currentToken = tokenizer.getNextToken();
  while (currentToken == ",") {
    targetECU = assert_token(tokenizer, Token::Identifier);
    currentToken = tokenizer.getNextToken();
  }

  if (currentToken != Token::Eof)
    tokenizer.saveToken(currentToken);

  return CANSignal(
    signalName.image,
    startBit.toUInt(),
    length.toUInt(),
    scale.toDouble(),
    offset.toDouble(),
    signedness == "-" ? CANSignal::Signed : CANSignal::Unsigned,
    endianess == "0" ? CANSignal::BigEndian : CANSignal::LittleEndian,
    CANSignal::Range::fromString(min.image, max.image)
  );
}

CANFrame parseFrame(Tokenizer& tokenizer) {
  Token name;
  Token id;
  Token dlc;
  Token ecu;

  assert_current_token(tokenizer, "BO_");

  id = assert_token(tokenizer, Token::Number);
  name = assert_token(tokenizer, Token::Identifier);

  assert_token(tokenizer, ":");

  dlc = assert_token(tokenizer, Token::Number);
  ecu = assert_token(tokenizer, Token::Identifier);

  CANFrame result(
    name.image, id.toUInt(), dlc.toUInt());

  Token currentToken = tokenizer.getNextToken();

  while(currentToken == "SG_") {
    result.addSignal(parseSignal(tokenizer));
    currentToken = tokenizer.getNextToken();
  }

  if(currentToken != Token::Eof)
    tokenizer.saveToken(currentToken);

  return result;
}

std::set<std::string> parseECUs(Tokenizer& tokenizer) {
  std::set<std::string> result;

  assert_current_token(tokenizer, "BU_");
  assert_token(tokenizer, ":");
  unsigned long long currentLine = tokenizer.lineCount();

  Token currentToken = assert_token(tokenizer, Token::Identifier);

  // Looking for all the identifiers on the same line
  while(currentToken != Token::Eof &&
        currentLine == tokenizer.lineCount()) {
    result.insert(currentToken.image);
    currentToken = assert_token(tokenizer, Token::Identifier);
  }

  if(currentToken != Token::Eof)
    tokenizer.saveToken(currentToken);

  return result;
}

void parseSignalChoices(Tokenizer& tokenizer, CANDatabase& db) {
  Token targetFrame;
  Token targetSignal;
  Token currentVal;
  Token currentLabel;

  std::map<unsigned int, std::string> targetChoices;

  assert_current_token(tokenizer, "VAL_");
  targetFrame = assert_token(tokenizer, Token::Number);
  targetSignal = assert_token(tokenizer, Token::Identifier);

  Token currentToken = tokenizer.getNextToken();
  while(currentToken != ";" &&
        currentToken != Token::Eof) {
    currentVal = assert_current_token(tokenizer, Token::Number);
    currentLabel = assert_token(tokenizer, Token::StringLiteral);

    targetChoices.insert(
      std::make_pair(
        currentVal.toUInt(),
        currentLabel.image
      )
    );

    currentToken = tokenizer.getNextToken();
  }

  assert_current_token(tokenizer, ";");

  unsigned long long frame_id = targetFrame.toUInt();
  if(!db.contains(frame_id) ||
     !db.at(frame_id).contains(targetSignal.image)) {
    warning("Cannot assign enum to signal \"" + targetFrame.image + "/" +
            targetSignal.image + "\"",
            tokenizer.lineCount());
    return;
  }

  db.at(frame_id)
    .at(targetSignal.image)
    .setChoices(targetChoices);
}
