#include "ParsingUtils.h"
#include <sstream>
#include <iomanip>

namespace dtl = CppCAN::parser::details;

std::string assert_token_str_err(
  const std::string& expected, const std::string& actual) {
 
  std::stringstream ss;
  ss << "Expected " << std::quoted(expected)
     << " but got " << std::quoted(actual);
  return ss.str(); 
}

std::string assert_token_type_err(
  dtl::Token::Type expected, const std::string& actual) {
  
  std::string targetTypeStr = "";

  using Token = dtl::Token;
  switch(expected) {
  case Token::Number:
    targetTypeStr = "NUMBER";
    break;
  case Token::PositiveNumber:
    targetTypeStr = "POSITIVE NUMBER";
    break;
  case Token::NegativeNumber:
    targetTypeStr = "NEGATIVE NUMBER";
    break;
  case Token::FloatingPointNumber:
    targetTypeStr = "FLOATING-POINT NUMBER";
    break;
  case Token::StringLiteral:
    targetTypeStr = "STRING LITERAL";
    break;
  case Token::Separator:
    targetTypeStr = "SEPARATOR";
    break;
  case Token::ArithmeticSign:
    targetTypeStr = "ARITHMETIC SIGN";
    break;
  case Token::Identifier:
    targetTypeStr = "IDENTIFIER";
    break;
  case Token::Eof:
    targetTypeStr = "EOF";
    break;
  }

  std::stringstream ss;
  ss << "Expected a(n) " << targetTypeStr
     << " but got " << std::quoted(actual);
  return ss.str(); 
}

bool dtl::is_token(dtl::Tokenizer& tokenizer, const std::string& token) {
  return tokenizer.getNextToken() == token;
}

bool dtl::is_token(dtl::Tokenizer& tokenizer, dtl::Token::Type token) {
  return tokenizer.getNextToken() == token;
}

bool dtl::is_current_token(const dtl::Tokenizer& tokenizer, const std::string& token) {
  return tokenizer.getCurrentToken() == token;
}

bool dtl::is_current_token(const dtl::Tokenizer& tokenizer, Token::Type token) {
  return tokenizer.getCurrentToken() == token;
}

const dtl::Token&
dtl::assert_token(dtl::Tokenizer& tokenizer, const std::string& token) {
  if(!dtl::is_token(tokenizer, token)) {
    dtl::throw_error(
        "Syntax error",
        assert_token_str_err(token, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const dtl::Token&
dtl::assert_token(dtl::Tokenizer& tokenizer, dtl::Token::Type type) {
  if(!dtl::is_token(tokenizer, type)) {
    dtl::throw_error(
        "Syntax error",
        assert_token_type_err(type, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const dtl::Token&
dtl::assert_current_token(const dtl::Tokenizer& tokenizer, const std::string& token) {
  if(!dtl::is_current_token(tokenizer, token)) {
    dtl::throw_error(
        "Syntax error",
        assert_token_str_err(token, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const dtl::Token&
dtl::assert_current_token(const Tokenizer& tokenizer, dtl::Token::Type type) {
  if(!dtl::is_current_token(tokenizer, type)) {
    dtl::throw_error(
        "Syntax error",
        assert_token_type_err(type, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

bool dtl::peek_token(dtl::Tokenizer& tokenizer, dtl::Token::Type type) {
  const dtl::Token& toCheck = tokenizer.getNextToken();
  if(toCheck == type) {
    return true;
  }

  tokenizer.saveToken(toCheck);
  return false;
}

bool dtl::peek_token(dtl::Tokenizer& tokenizer, const std::string& type) {
  const dtl::Token& toCheck = tokenizer.getNextToken();
  if(toCheck == type) {
    return true;
  }

  tokenizer.saveToken(toCheck);
  return false;
}

void dtl::throw_error(const std::string& category, const std::string& description,
                      unsigned long long line) {
  throw CppCAN::CANDatabaseException(
    category + ": " + description + " at line " + std::to_string(line + 1)
  );
}

void dtl::warning(std::vector<CppCAN::CANDatabase::parsing_warning>* warnings, 
                  const std::string& description, unsigned long long line) {
  if(warnings == nullptr)
    return;

  warnings->push_back({ line, description });
}