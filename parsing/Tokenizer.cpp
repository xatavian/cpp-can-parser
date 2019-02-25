#include "Tokenizer.h"
#include "models/CANDatabaseException.h"
#include <cctype>
#include <iostream>

char Tokenizer::getNextChar() {
  if(eofReached)
    return 0;

  char result;
  inputstream.get(result);

  charCnt += 1;
  if(addLine) {
    lineCnt += 1;
    addLine = false;
  }
  if(result == '\n') {
    addLine = true;
  }

  if(inputstream.eof())
    eofReached = true;

  return result;
}

unsigned long long Tokenizer::charCount() const {
  return charCnt;
}

unsigned long long Tokenizer::lineCount() const {
  return lineCnt;
}

bool Tokenizer::isSpace(char c) const {
  return  std::isspace( (unsigned char) c);
}

bool Tokenizer::isSeparator(char c) const {
  return c == '[' || c == ']' || c == '|' || c == ':' || c == '@' ||
         c == ')' || c == '(' || c == ',' || c == ';';
}

bool Tokenizer::isDigit(char c) const {
  return std::isdigit( (unsigned char) c);
}

bool Tokenizer::isIdentifierStart(char c) const {
  return std::isalpha( (unsigned char) c) || c == '_';
}

bool Tokenizer::isIdentifierPart(char c) const {
  return isIdentifierStart(c) || isDigit(c);
}

bool Tokenizer::isEOF(char c) const {
  return c == '\0';
}

Token Tokenizer::getCurrentToken() const {
  return currentToken;
}

void Tokenizer::saveToken(const Token& token) {
  tokenStack.push_back(token);
}

Token Tokenizer::getNextToken() {
  if(tokenStack.size() > 0) {
    Token result = tokenStack.back();
    tokenStack.pop_back();
    return result;
  }

  if(!started) {
    currentChar = getNextChar();
    started = true;
  }

  while(isSpace(currentChar))
    currentChar = getNextChar();

  if(isEOF(currentChar)) {
    currentToken = Token(Token::Eof, "");
  }
  else if(isSeparator(currentChar)) {
    currentToken = Token(Token::Separator, std::string(1, currentChar));
    currentChar = getNextChar();
  }
  else if(currentChar == '+') {
    currentToken = Token(Token::Sign, std::string(1, currentChar));
    currentChar = getNextChar();
  }
  else if(currentChar == '-') {
    currentChar = getNextChar();
    if(!isDigit(currentChar))
      currentToken = Token(Token::Sign, "-");
    else { // Negative number
      std::string literal = "-";
      while(isDigit(currentChar) || currentChar == '.') {
        literal += currentChar;
        currentChar = getNextChar();
      }
      currentToken = Token(Token::Number, literal);
    }
  }
  else if(currentChar == '\"') {
    std::string literal = ""; // std::string(1, currentChar);
    currentChar = getNextChar();

    while(currentChar != '\"' && !isEOF(currentChar)) {
      literal += currentChar;
      currentChar = getNextChar();
    }

    if(currentChar == '\"') {
      // literal += currentChar;
      currentChar = getNextChar();
    }
    else {
      std::cout << "Error in literal parsing: reached EOF before literal end" << std::endl;
    }

    currentToken = Token(Token::Literal, literal);
  }
  else if(isDigit(currentChar)) {
    std::string literal = std::string(1, currentChar);
    currentChar = getNextChar();

    while(isDigit(currentChar) || currentChar == '.') {
      literal += currentChar;
      currentChar = getNextChar();
    }

    currentToken = Token(Token::Number, literal);
  }
  else if(isIdentifierStart(currentChar)) {
    std::string identifier = std::string(1, currentChar);
    currentChar = getNextChar();

    while(isIdentifierPart(currentChar)) {
      identifier += currentChar;
      currentChar = getNextChar();
    }

    currentToken = Token(Token::Identifier, identifier);
  }
  else {
    throw CANDatabaseException(
      "Invalid character \"" +
      std::string(1, currentChar) +
      "\" encountered at line " +
      std::to_string(lineCount())
    );
  }

  // std::cout << "Token: " << currentToken.image() << std::endl;
  return currentToken;
}

void Tokenizer::skipLine() {
  while(currentChar != '\n' && currentChar != '\0') {
    currentChar = getNextChar();

    // If we were in the middle of a literal, ignore the whole
    // literal even if it spans over several lines
    if(currentChar == '\"') {
      currentChar = getNextChar();
      while(currentChar != '\"' && currentChar != '\0')
        currentChar = getNextChar();
    }
  }
}

void Tokenizer::skipUntil(const std::string& token) {
  unsigned long long initLine = lineCount();

  while(currentToken.image() != token &&
        currentToken.type() != Token::Eof) {
    getNextToken();
  }

  if(currentToken.type() == Token::Eof) {
    throw CANDatabaseException(
      "Error: due to an unrecognized (and badly formed) command at line " +
      std::to_string(initLine) +
      " the parsing has failed"
    );
  }
}
