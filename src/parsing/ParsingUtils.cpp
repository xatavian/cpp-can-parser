#include "ParsingUtils.h"
#include <sstream>
#include <iomanip>

std::string assert_token_str_err(
  const std::string& expected, const std::string& actual) {
 
  std::stringstream ss;
  ss << "Expected " << std::quoted(expected)
     << " but got " << std::quoted(actual);
  return ss.str(); 
}

std::string assert_token_type_err(
  Token::Type expected, const std::string& actual) {
  
  std::string targetTypeStr = "";
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

bool is_token(Tokenizer& tokenizer, const std::string& token) {
  return tokenizer.getNextToken() == token;
}

bool is_token(Tokenizer& tokenizer, Token::Type token) {
  return tokenizer.getNextToken() == token;
}

bool is_current_token(const Tokenizer& tokenizer, const std::string& token) {
  return tokenizer.getCurrentToken() == token;
}

bool is_current_token(const Tokenizer& tokenizer, Token::Type token) {
  return tokenizer.getCurrentToken() == token;
}

const Token&
assert_token(Tokenizer& tokenizer, const std::string& token) {
  if(!is_token(tokenizer, token)) {
    throw_error(
        "Syntax error",
        assert_token_str_err(token, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const Token&
assert_token(Tokenizer& tokenizer, Token::Type type) {
  if(!is_token(tokenizer, type)) {
    throw_error(
        "Syntax error",
        assert_token_type_err(type, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const Token&
assert_current_token(const Tokenizer& tokenizer, const std::string& token) {
  if(!is_current_token(tokenizer, token)) {
    throw_error(
        "Syntax error",
        assert_token_str_err(token, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

const Token&
assert_current_token(const Tokenizer& tokenizer, Token::Type type) {
  if(!is_current_token(tokenizer, type)) {
    throw_error(
        "Syntax error",
        assert_token_type_err(type, tokenizer.getCurrentToken().image),
        tokenizer.lineCount());
  }

  return tokenizer.getCurrentToken();
}

bool peek_token(Tokenizer& tokenizer, Token::Type type) {
  const Token& toCheck = tokenizer.getNextToken();
  if(toCheck == type) {
    return true;
  }

  tokenizer.saveToken(toCheck);
  return false;
}

bool peek_token(Tokenizer& tokenizer, const std::string& type) {
  const Token& toCheck = tokenizer.getNextToken();
  if(toCheck == type) {
    return true;
  }

  tokenizer.saveToken(toCheck);
  return false;
}

void throw_error(const std::string& category, const std::string& description,
                unsigned long long line) {
  throw CANDatabaseException(
    category + ": " + description + " at line " + std::to_string(line + 1)
  );
}

void warning(const std::string& description, unsigned long long line) {
  std::cerr << "WARNING: "
            << description
            << " at line "
            << line + 1
            << std::endl;
}