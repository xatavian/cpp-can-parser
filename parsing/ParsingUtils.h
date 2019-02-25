#ifndef ParsingUtils_H
#define ParsingUtils_H

#include "parsing/Tokenizer.h"
#include <string>
#include <iostream>
#include "models/CANDatabaseException.h"

void throwError(const std::string& category, const std::string& description,
                unsigned long long line) {
  throw CANDatabaseException(
    category + ": " + description + " at line " + std::to_string(line)
  );
}

void warning(const std::string& description, unsigned long long line) {
  std::cout << "WARNING: "
            << description
            << " at line "
            << line
            << std::endl;
}

void skipIf(Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getNextToken().image() == token)
    return;

  throwError(
    "Syntax error",
    "expected \"" + token + "\" but got \"" +
      tokenizer.getCurrentToken().image() + "\"",
    tokenizer.lineCount()
  );
}

void assertToken(const Tokenizer& tokenizer, const std::string& token) {
  if(tokenizer.getCurrentToken().image() == token)
    return;

  throwError(
    "Syntax error",
    "expected \"" + token + "\" but got \"" +
      tokenizer.getCurrentToken().image() + "\"",
    tokenizer.lineCount()
  );
}

Token checkTokenType(Tokenizer& tokenizer, Token::Type targetType) {
  Token result = tokenizer.getNextToken();
  if(result.type() == targetType)
    return result;

  throwError(
    "Syntax error",
    "unexpected \"" + result.image() + "\"",
    tokenizer.lineCount()
  );

  // Never reached but it removes a compilation warning
  return result;
}


#endif
