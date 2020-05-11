#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include "ParsingUtils.h"
#include "DBCParser.h"

using namespace CppCAN::parser::dbc;
namespace dtl = CppCAN::parser::details;

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

static bool 
is_dbc_token(const dtl::Token& token) {
  return SUPPORTED_DBC_TOKENS.count(token.image) > 0 ||
         NS_TOKENS.count(token.image) > 0||
         UNSUPPORTED_DBC_TOKENS.count(token.image) > 0;
}

CppCAN::CANDatabase 
CppCAN::parser::dbc::fromTokenizer(dtl::Tokenizer& tokenizer, 
                                   std::vector<CANDatabase::parsing_warning>* warnings) {
  return fromTokenizer("", tokenizer, warnings);
}

static std::string
parseVersionSection(dtl::Tokenizer& tokenizer) {
  if(dtl::peek_token(tokenizer, VERSION_TOKEN)) {
    dtl::Token candb_version = assert_token(tokenizer, dtl::Token::StringLiteral);
    // std::cout << "CANdb++ version: " << candb_version.image << std::endl;
    return candb_version.image;
  }

  return "";
}

static void
parseNSSection(dtl::Tokenizer& tokenizer) {
  if(!dtl::peek_token(tokenizer, NS_SECTION_TOKEN1) && 
     !dtl::peek_token(tokenizer, NS_SECTION_TOKEN2)) // Sometimes, one can find both NS_ ans _NS in DBC files
    return;

  dtl::assert_token(tokenizer, ":");
  
  dtl::Token token = tokenizer.getNextToken();
  while (NS_TOKENS.count(token.image) > 0) {
    token = tokenizer.getNextToken();
  }

  tokenizer.saveTokenIfNotEof(token);
}

static void
parseBitTimingSection(dtl::Tokenizer& tokenizer) {
  assert_token(tokenizer, BIT_TIMING_TOKEN);
  assert_token(tokenizer, ":");

  if (peek_token(tokenizer, dtl::Token::PositiveNumber)) {
    dtl::Token baudrate = dtl::assert_current_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::assert_token(tokenizer, ":");
    dtl::Token btr1 = assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::assert_token(tokenizer, ",");
    dtl::Token btr2 = assert_token(tokenizer, dtl::Token::PositiveNumber);
  }
}

static void
parseNodesSection(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                  std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  dtl::assert_token(tokenizer, NODE_DEF_TOKEN);
  dtl::assert_token(tokenizer, ":");

  std::set<std::string> nodes;

  if(!dtl::peek_token(tokenizer, dtl::Token::Identifier)) {
    return;
  }

  dtl::Token currentToken = dtl::assert_token(tokenizer, dtl::Token::Identifier);

  // Looking for all the identifiers on the same line
  while(currentToken != dtl::Token::Eof &&
        !is_dbc_token(currentToken)) {
    
    if(nodes.count(currentToken.image) > 0) {
      dtl::warning(warnings, currentToken.image + " is an already registered node name", 
              tokenizer.lineCount());
    }
    else {
      nodes.insert(currentToken.image);
    }
    
    currentToken = dtl::assert_token(tokenizer, dtl::Token::Identifier);
  }

  tokenizer.saveTokenIfNotEof(currentToken);
}

static void
parseUnsupportedCommandSection(dtl::Tokenizer& tokenizer, const std::string& command, 
                               std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  while(dtl::peek_token(tokenizer, command)) {
    // In DBC files, some instructions don't finish by a semi-colon.
    // Fotunately, all the unsupported ones do finish by a semi-colon.
    dtl::warning(
      warnings, 
      "Skipped \"" + command + "\" instruction "
      "because it is not supported", 
      tokenizer.lineCount()); 
    tokenizer.skipUntil(";");
  }
}

static void
parseSigDefInstruction(dtl::Tokenizer& tokenizer, CppCAN::CANFrame& frame, 
                       std::vector<CppCAN::CANDatabase::parsing_warning>* warnings ) {
  dtl::assert_current_token(tokenizer, SIG_DEF_TOKEN);

  dtl::Token name = dtl::assert_token(tokenizer, dtl::Token::Identifier);
  dtl::assert_token(tokenizer, ":");
  dtl::Token startBit = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
  dtl::assert_token(tokenizer, "|");
  dtl::Token length = assert_token(tokenizer, dtl::Token::PositiveNumber);
  dtl::assert_token(tokenizer, "@");
  dtl::Token endianess = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
  dtl::Token signedness = dtl::assert_token(tokenizer, dtl::Token::ArithmeticSign);
  dtl::assert_token(tokenizer, "(");
  dtl::Token scale = dtl::assert_token(tokenizer, dtl::Token::Number);
  dtl::assert_token(tokenizer, ",");
  dtl::Token offset = assert_token(tokenizer, dtl::Token::Number);
  dtl::assert_token(tokenizer, ")");
  dtl::assert_token(tokenizer, "[");
  dtl::Token min = dtl::assert_token(tokenizer, dtl::Token::Number);
  dtl::assert_token(tokenizer, "|");
  dtl::Token max = dtl::assert_token(tokenizer, dtl::Token::Number);
  dtl::assert_token(tokenizer, "]");
  dtl::Token unit = dtl::assert_token(tokenizer, dtl::Token::StringLiteral);
   
  // ECU are ignored for now
  dtl::Token targetECU = dtl::assert_token(tokenizer, dtl::Token::Identifier);  
  while (dtl::peek_token(tokenizer, ",")) {
    targetECU = dtl::assert_token(tokenizer, dtl::Token::Identifier);
  }

  if(frame.contains(name.image)) {
    std::stringstream ss;
    ss << "Double declaration of the signal " << std::quoted(name.image)
       << " in frame " << frame.can_id();  
    dtl::warning(warnings, ss.str(), tokenizer.lineCount());
  }

  frame.addSignal(
    CppCAN::CANSignal(
      name.image,
      startBit.toUInt(),
      length.toUInt(),
      scale.toDouble(),
      offset.toDouble(),
      signedness == "-" ? CppCAN::CANSignal::Signed : CppCAN::CANSignal::Unsigned,
      endianess == "0" ? CppCAN::CANSignal::BigEndian : CppCAN::CANSignal::LittleEndian,
      CppCAN::CANSignal::Range::fromString(min.image, max.image)
    )
  );
}

static void
parseMsgDefSection(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                   std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  while(dtl::peek_token(tokenizer, MESSAGE_DEF_TOKEN)) {
    dtl::Token id = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::Token name = dtl::assert_token(tokenizer, dtl::Token::Identifier);

    dtl::assert_token(tokenizer, ":");

    dtl::Token dlc = assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::Token ecu = assert_token(tokenizer, dtl::Token::Identifier);

    if(db.contains(id.toUInt())) {
      dtl::throw_error("Database error", "Double declaration of frame with CAN ID " + id.image, tokenizer.lineCount());
    }

    if(db.contains(name.image)) {
      std::stringstream ss;
      ss << "Double declaration of the frame with name " << std::quoted(name.image);
      dtl::warning(warnings, ss.str(), tokenizer.lineCount());
    }

    CppCAN::CANFrame new_frame(
      name.image, id.toUInt(), dlc.toUInt());

    while(dtl::peek_token(tokenizer, SIG_DEF_TOKEN)) {
      parseSigDefInstruction(tokenizer, new_frame, warnings);
    }

    db.addFrame(new_frame);
  }
}

static void
parseMsgCommentInstruction(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                           std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  dtl::Token targetFrame = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
  dtl::Token comment = dtl::assert_token(tokenizer, dtl::Token::StringLiteral);
  dtl::assert_token(tokenizer, ";");

  auto frame_id = targetFrame.toUInt();
  if(db.contains(frame_id)) {
    db.at(frame_id).setComment(comment.image);
  }
  else {
    dtl::warning(
      warnings, 
      "Invalid comment instruction: Frame with "
      "id " + targetFrame.image + " does not exist", 
      tokenizer.lineCount());
  }
}

static void
parseSigCommentInstruction(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                           std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  dtl::Token targetFrame = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
  dtl::Token targetSignal = dtl::assert_token(tokenizer, dtl::Token::Identifier);
  dtl::Token comment = dtl::assert_token(tokenizer, dtl::Token::StringLiteral);
  dtl::assert_token(tokenizer, ";");

  if(!db.contains(targetFrame.toUInt())) {
    dtl::warning(warnings, "Invalid comment instruction: Frame with id " + targetFrame.image + " does not exist", tokenizer.lineCount());
    return;
  }

  CppCAN::CANFrame& frame = db[targetFrame.toUInt()];
  if(!frame.contains(targetSignal.image)) {
    dtl::warning(
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
parseCommentSection(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                    std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  while(dtl::peek_token(tokenizer, COMMENT_TOKEN)) {
    if(dtl::peek_token(tokenizer, dtl::Token::StringLiteral)) {
      // TODO: handle global comment
      dtl::assert_token(tokenizer, ";");
      dtl::warning(warnings, "Unsupported comment instruction", tokenizer.lineCount());
      continue;
    }

    dtl::Token commentType = dtl::assert_token(tokenizer, dtl::Token::Identifier);
    if(commentType == MESSAGE_DEF_TOKEN) {
      parseMsgCommentInstruction(tokenizer, db, warnings);
    }
    else if(commentType == SIG_DEF_TOKEN) {
      parseSigCommentInstruction(tokenizer, db, warnings);
    }
    else {
      dtl::warning(warnings, "Unsupported comment instruction", tokenizer.lineCount());
      tokenizer.skipUntil(";");
    }
  }
}

static void
parseAttrValSection(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                    std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  while(dtl::peek_token(tokenizer, ATTR_VAL_TOKEN)) {
    dtl::Token attrType = dtl::assert_token(tokenizer, dtl::Token::StringLiteral);

    if(attrType != "GenMsgCycleTime" && attrType != "CycleTime") {
      tokenizer.skipUntil(";");
      dtl::warning(warnings, "Unsupported BA_ operation", tokenizer.lineCount());
      continue;
    }
 
    dtl::assert_token(tokenizer, "BO_");
    dtl::Token frameId = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::Token period = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::assert_token(tokenizer, ";");

    try {
      db[frameId.toUInt()].setPeriod(period.toUInt());
    }
    catch (const std::out_of_range& e) {
     dtl::warning(warnings, frameId.image + " does not exist", tokenizer.lineCount());
    }
  }
}

static void
parseValDescSection(dtl::Tokenizer& tokenizer, CppCAN::CANDatabase& db, 
                    std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
  while(dtl::peek_token(tokenizer, SIG_VAL_DEF_TOKEN)) {
    dtl::Token targetFrame = dtl::assert_token(tokenizer, dtl::Token::PositiveNumber);
    dtl::Token targetSignal = dtl::assert_token(tokenizer, dtl::Token::Identifier);
    
    std::map<unsigned int, std::string> targetChoices;

    while(!dtl::peek_token(tokenizer, ";")) {
      dtl::Token value = dtl::assert_token(tokenizer, dtl::Token::Number);
      dtl::Token desc = dtl::assert_token(tokenizer, dtl::Token::StringLiteral);

      targetChoices.insert(std::make_pair(value.toUInt(), desc.image));
    }

    if(!db.contains(targetFrame.toUInt())) {
      dtl::warning(
        warnings, 
        "Invalid VAL_ instruction: Frame with id " + 
        targetFrame.image + " does not exist", 
        tokenizer.lineCount());
      continue;
    }

    CppCAN::CANFrame& frame = db[targetFrame.toUInt()];
    if(!frame.contains(targetSignal.image)) {
      dtl::warning(
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


CppCAN::CANDatabase
CppCAN::parser::dbc::fromTokenizer(const std::string& name, dtl::Tokenizer& tokenizer, std::vector<CppCAN::CANDatabase::parsing_warning>* warnings) {
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

  while(!dtl::is_token(tokenizer, dtl::Token::Eof)) {
    // We have a syntax error because we have a token which does not
    // represent any command.
    if(!is_dbc_token(tokenizer.getCurrentToken())) {
      dtl::throw_error("Syntax error", 
                       "Unexpected token \"" + tokenizer.getCurrentToken().image + "\"", 
                       tokenizer.lineCount());
    }

    // We have a valid DBC instruction, but we ignore it because
    // it is not in a valid position.
    dtl::warning(
      warnings,
      "Unexpected token " + tokenizer.getCurrentToken().image + 
      " at line " + std::to_string(tokenizer.lineCount())     +
      " (maybe is it an unsupported instruction ? maybe is it a misplaced instruction ?)",
      tokenizer.lineCount());
    tokenizer.skipUntil(";");
  }

  return result;
}