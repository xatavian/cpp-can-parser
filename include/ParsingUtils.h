#ifndef ParsingUtils_H
#define ParsingUtils_H

#include "Tokenizer.h"
#include <string>
#include <iostream>
#include "CANDatabaseException.h"

void throwError(const std::string& category, const std::string& description,
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

void skipIf(Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getNextToken() == token)
    return;

  throwError(
    "Syntax error",
    "expected \"" + token + "\" but got \"" +
    tokenizer.getCurrentToken().image + "\"",
    tokenizer.lineCount()
  );
}

void assertToken(const Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getCurrentToken() == token)
    return;

  throwError(
    "Syntax error",
    "expected \"" + token + "\" but got \"" +
    tokenizer.getCurrentToken().image + "\"",
    tokenizer.lineCount()
  );
}


Token checkCurrentTokenType(const Token& toCheck, Token::Type targetType,
                            unsigned long long line) {
  if(toCheck == targetType)
    return toCheck;

  throwError(
    "Syntax error",
    "unexpected \"" + toCheck.image  + "\"",
    line
  );

  // Removes a compilation warning
  return toCheck;
}

Token checkCurrentTokenType(const Token& toCheck,
                            const std::string& token,
                            unsigned long long line) {
  if(toCheck == token)
    return toCheck;

  throwError(
    "Syntax error",
    "unexpected \"" + toCheck.image + "\"",
    line
  );

  // Removes a compilation warning
  return toCheck;
}

Token checkTokenType(Tokenizer& tokenizer, Token::Type targetType) {
  return checkCurrentTokenType(tokenizer.getNextToken(),
                               targetType,
                               tokenizer.lineCount());
}
#endif
