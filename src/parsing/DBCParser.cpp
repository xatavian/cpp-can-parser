#include "DBCParser.h"
#include "CANDatabaseException.h"
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include "ParsingUtils.h"

using namespace DBCParser;

static std::string VERSION_TOKEN = "VERSION";
static std::string NS_SECTION_TOKEN1 = "NS_";
static std::string NS_SECTION_TOKEN2 = "_NS";
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
  "EV_DATA_", "ENVVAR_DATA_", "SGTYPE_", "SGTYPE_VAL_", "BA_DEF_SGTYPE_", "BA_SGTYPE_",
  "SIG_TYPE_DEF_", "SIG_TYPE_REF_", "VAL_TABLE_", "SIG_GROUP_", "SIG_VALTYPE_",
  "SIGTYPE_VALTYPE_", "BO_TX_BU_", "BA_DEF_REL_", "BA_REL_", "BA_DEF_DEF_REL_", 
  "BU_SG_REL_", "BU_EV_REL_", "BU_BO_REL_"
};

static std::set<std::string> UNSUPPORTED_DBC_TOKENS = {
  "VAL_TABLE_", "BO_TX_BU_", "ENVVAR_DATA_",
  "SGTYPE_", "SIG_GROUP_"
}; 

bool is_dbc_token(const Token& token) {
  return SUPPORTED_DBC_TOKENS.count(token.image) > 0 ||
         NS_TOKENS.count(token.image) > 0||
         UNSUPPORTED_DBC_TOKENS.count(token.image) > 0;
}

CANDatabase DBCParser::fromTokenizer(Tokenizer& tokenizer, std::vector<CANDatabase::parsing_warning>* warnings) {
  return fromTokenizer("", tokenizer, warnings);
}

std::string parseVersionSection(Tokenizer& tokenizer) {
  if(peek_token(tokenizer, VERSION_TOKEN)) {
    Token candb_version = assert_token(tokenizer, Token::StringLiteral);
    // std::cout << "CANdb++ version: " << candb_version.image << std::endl;
    return candb_version.image;
  }

  return "";
}

static void
parseNSSection(Tokenizer& tokenizer) {
  if(!peek_token(tokenizer, NS_SECTION_TOKEN1) && 
     !peek_token(tokenizer, NS_SECTION_TOKEN2)) // Sometimes, one can find both NS_ ans _NS in DBC files
    return;

  assert_token(tokenizer, ":");
  
  Token token = tokenizer.getNextToken();
  while (NS_TOKENS.count(token.image) > 0) {
    token = tokenizer.getNextToken();
  }

  tokenizer.saveTokenIfNotEof(token);
}

static void
parseBitTimingSection(Tokenizer& tokenizer) {
  assert_token(tokenizer, BIT_TIMING_TOKEN);
  assert_token(tokenizer, ":");

  if (peek_token(tokenizer, Token::PositiveNumber)) {
    Token baudrate = assert_current_token(tokenizer, Token::PositiveNumber);
    assert_token(tokenizer, ":");
    Token btr1 = assert_token(tokenizer, Token::PositiveNumber);
    assert_token(tokenizer, ",");
    Token btr2 = assert_token(tokenizer, Token::PositiveNumber);
  }
}

static void
parseNodesSection(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  assert_token(tokenizer, NODE_DEF_TOKEN);
  assert_token(tokenizer, ":");

  std::set<std::string> nodes;

  if(!peek_token(tokenizer, Token::Identifier)) {
    return;
  }

  Token currentToken = assert_token(tokenizer, Token::Identifier);

  // Looking for all the identifiers on the same line
  while(currentToken != Token::Eof &&
        !is_dbc_token(currentToken)) {
    
    if(nodes.count(currentToken.image) > 0) {
      warning(warnings, currentToken.image + " is an already registered node name", 
              tokenizer.lineCount());
    }
    else {
      nodes.insert(currentToken.image);
    }
    
    currentToken = assert_token(tokenizer, Token::Identifier);
  }

  tokenizer.saveTokenIfNotEof(currentToken);
}

static void
parseUnsupportedCommandSection(Tokenizer& tokenizer, const std::string& command, std::vector<CANDatabase::parsing_warning>* warnings) {
  while(peek_token(tokenizer, command)) {
    // In DBC files, some instructions don't finish by a semi-colon.
    // Fotunately, all the unsupported ones do finish by a semi-colon.
    warning(
      warnings, 
      "Skipped \"" + command + "\" instruction "
      "because it is not supported", 
      tokenizer.lineCount()); 
    tokenizer.skipUntil(";");
  }
}

static void
parseSigDefInstruction(Tokenizer& tokenizer, CANFrame& frame, std::vector<CANDatabase::parsing_warning>* warnings ) {
  assert_current_token(tokenizer, SIG_DEF_TOKEN);

  Token name = assert_token(tokenizer, Token::Identifier);
  assert_token(tokenizer, ":");
  Token startBit = assert_token(tokenizer, Token::PositiveNumber);
  assert_token(tokenizer, "|");
  Token length = assert_token(tokenizer, Token::PositiveNumber);
  assert_token(tokenizer, "@");
  Token endianess = assert_token(tokenizer, Token::PositiveNumber);
  Token signedness = assert_token(tokenizer, Token::ArithmeticSign);
  assert_token(tokenizer, "(");
  Token scale = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, ",");
  Token offset = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, ")");
  assert_token(tokenizer, "[");
  Token min = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "|");
  Token max = assert_token(tokenizer, Token::Number);
  assert_token(tokenizer, "]");
  Token unit = assert_token(tokenizer, Token::StringLiteral);
   
  // ECU are ignored for now
  Token targetECU = assert_token(tokenizer, Token::Identifier);  
  while (peek_token(tokenizer, ",")) {
    targetECU = assert_token(tokenizer, Token::Identifier);
  }

  if(frame.contains(name.image)) {
    std::stringstream ss;
    ss << "Double declaration of the signal " << std::quoted(name.image)
       << " in frame " << frame.can_id();  
    warning(warnings, ss.str(), tokenizer.lineCount());
  }

  frame.addSignal(
    CANSignal(
      name.image,
      startBit.toUInt(),
      length.toUInt(),
      scale.toDouble(),
      offset.toDouble(),
      signedness == "-" ? CANSignal::Signed : CANSignal::Unsigned,
      endianess == "0" ? CANSignal::BigEndian : CANSignal::LittleEndian,
      CANSignal::Range::fromString(min.image, max.image)
    )
  );
}

static void
parseMsgDefSection(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  while(peek_token(tokenizer, MESSAGE_DEF_TOKEN)) {
    Token id = assert_token(tokenizer, Token::PositiveNumber);
    Token name = assert_token(tokenizer, Token::Identifier);

    assert_token(tokenizer, ":");

    Token dlc = assert_token(tokenizer, Token::PositiveNumber);
    Token ecu = assert_token(tokenizer, Token::Identifier);

    if(db.contains(id.toUInt())) {
      throw_error("Database error", "Double declaration of frame with CAN ID " + id.image, tokenizer.lineCount());
    }

    if(db.contains(name.image)) {
      std::stringstream ss;
      ss << "Double declaration of the frame with name " << std::quoted(name.image);
      warning(warnings, ss.str(), tokenizer.lineCount());
    }

    CANFrame new_frame(
      name.image, id.toUInt(), dlc.toUInt());

    while(peek_token(tokenizer, SIG_DEF_TOKEN)) {
      parseSigDefInstruction(tokenizer, new_frame, warnings);
    }

    db.addFrame(new_frame);
  }
}

static void
parseMsgCommentInstruction(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  Token targetFrame = assert_token(tokenizer, Token::PositiveNumber);
  Token comment = assert_token(tokenizer, Token::StringLiteral);
  assert_token(tokenizer, ";");

  auto frame_id = targetFrame.toUInt();
  if(db.contains(frame_id)) {
    db.at(frame_id).setComment(comment.image);
  }
  else {
    warning(
      warnings, 
      "Invalid comment instruction: Frame with "
      "id " + targetFrame.image + " does not exist", 
      tokenizer.lineCount());
  }
}

static void
parseSigCommentInstruction(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  Token targetFrame = assert_token(tokenizer, Token::PositiveNumber);
  Token targetSignal = assert_token(tokenizer, Token::Identifier);
  Token comment = assert_token(tokenizer, Token::StringLiteral);
  assert_token(tokenizer, ";");

  if(!db.contains(targetFrame.toUInt())) {
    warning(warnings, "Invalid comment instruction: Frame with id " + targetFrame.image + " does not exist", tokenizer.lineCount());
    return;
  }

  CANFrame& frame = db[targetFrame.toUInt()];
  if(!frame.contains(targetSignal.image)) {
    warning(
      warnings, 
      "Invalid comment instruction: Frame with "
      "id " + targetFrame.image + " does not have a signal "
      "named \"" + targetSignal.image + "\"", 
      tokenizer.lineCount()
    );
  }
  else {
    frame[targetSignal.image].setComment(comment.image);
  }

}

static void
parseCommentSection(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  while(peek_token(tokenizer, COMMENT_TOKEN)) {
    if(peek_token(tokenizer, Token::StringLiteral)) {
      // TODO: handle global comment
      assert_token(tokenizer, ";");
      warning(warnings, "Unsupported comment instruction", tokenizer.lineCount());
      continue;
    }

    Token commentType = assert_token(tokenizer, Token::Identifier);
    if(commentType == MESSAGE_DEF_TOKEN) {
      parseMsgCommentInstruction(tokenizer, db, warnings);
    }
    else if(commentType == SIG_DEF_TOKEN) {
      parseSigCommentInstruction(tokenizer, db, warnings);
    }
    else {
      warning(warnings, "Unsupported comment instruction", tokenizer.lineCount());
      tokenizer.skipUntil(";");
    }
  }
}

static void
parseAttrValSection(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  while(peek_token(tokenizer, ATTR_VAL_TOKEN)) {
    Token attrType = assert_token(tokenizer, Token::StringLiteral);

    if(attrType != "GenMsgCycleTime" && attrType != "CycleTime") {
      tokenizer.skipUntil(";");
      warning(warnings, "Unsupported BA_ operation", tokenizer.lineCount());
      continue;
    }
 
    assert_token(tokenizer, "BO_");
    Token frameId = assert_token(tokenizer, Token::PositiveNumber);
    Token period = assert_token(tokenizer, Token::PositiveNumber);
    assert_token(tokenizer, ";");

    try {
      db[frameId.toUInt()].setPeriod(period.toUInt());
    }
    catch (const std::out_of_range& e) {
     warning(warnings, frameId.image + " does not exist", tokenizer.lineCount());
    }
  }
}

static void
parseValDescSection(Tokenizer& tokenizer, CANDatabase& db, std::vector<CANDatabase::parsing_warning>* warnings) {
  while(peek_token(tokenizer, SIG_VAL_DEF_TOKEN)) {
    Token targetFrame = assert_token(tokenizer, Token::PositiveNumber);
    Token targetSignal = assert_token(tokenizer, Token::Identifier);
    
    std::map<unsigned int, std::string> targetChoices;

    while(!peek_token(tokenizer, ";")) {
      Token value = assert_token(tokenizer, Token::Number);
      Token desc = assert_token(tokenizer, Token::StringLiteral);

      targetChoices.insert(std::make_pair(value.toUInt(), desc.image));
    }

    if(!db.contains(targetFrame.toUInt())) {
      warning(
        warnings, 
        "Invalid VAL_ instruction: Frame with id " + 
        targetFrame.image + " does not exist", 
        tokenizer.lineCount());
      continue;
    }

    CANFrame& frame = db[targetFrame.toUInt()];
    if(!frame.contains(targetSignal.image)) {
      warning(
        warnings, 
        "Invalid VAL_ instruction: Frame " + targetFrame.image + 
        " does not have a signal named \"" + targetSignal.image + "\"", 
        tokenizer.lineCount());
    }
    else {
      frame[targetSignal.image].setChoices(targetChoices);
    }
  }
}


CANDatabase DBCParser::fromTokenizer(const std::string& name, Tokenizer& tokenizer, std::vector<CANDatabase::parsing_warning>* warnings) {
  CANDatabase result(name);

  parseVersionSection(tokenizer);
  parseNSSection(tokenizer);
  parseBitTimingSection(tokenizer);
  parseNodesSection(tokenizer, result, warnings);
  parseUnsupportedCommandSection(tokenizer, "VAL_TABLE_", warnings);
  parseMsgDefSection(tokenizer, result, warnings);
  parseUnsupportedCommandSection(tokenizer, "BO_TX_BU_", warnings);
  parseUnsupportedCommandSection(tokenizer, "EV_", warnings);
  parseUnsupportedCommandSection(tokenizer, "SGTYPE_", warnings);
  parseCommentSection(tokenizer, result, warnings);
  parseUnsupportedCommandSection(tokenizer, "BA_DEF_", warnings);
  parseUnsupportedCommandSection(tokenizer, "SIG_VALTYPE_", warnings);
  parseUnsupportedCommandSection(tokenizer, "BA_DEF_DEF_", warnings);
  parseAttrValSection(tokenizer, result, warnings);
  parseValDescSection(tokenizer, result, warnings);

  while(!is_token(tokenizer, Token::Eof)) {
    // We have a syntax error because we have a token which does not
    // represent any command.
    if(!is_dbc_token(tokenizer.getCurrentToken())) {
      throw_error("Syntax error", 
                  "Unexpected token \"" + tokenizer.getCurrentToken().image + "\"", 
                  tokenizer.lineCount());
    }

    // We have a valid DBC instruction, but we ignore it because
    // it is not in a valid position.
    warning(
      warnings,
      "Unexpected token " + tokenizer.getCurrentToken().image + 
      " at line " + std::to_string(tokenizer.lineCount())     +
      " (maybe is it an unsupported instruction ? maybe is it a misplaced instruction ?)",
      tokenizer.lineCount());
    tokenizer.skipUntil(";");
  }

  return result;
}