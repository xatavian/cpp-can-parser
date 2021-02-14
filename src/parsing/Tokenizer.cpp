#include "Tokenizer.h"
#include "CANDatabase.h"
#include <cctype>
#include <iostream>

using namespace CppCAN::parser::details;

bool isSpace(char c) {
  return  std::isspace((unsigned char)c);
}

bool isSeparator(char c) {
  return c == '[' || c == ']' || c == '|' || c == ':' || c == '@' ||
    c == ')' || c == '(' || c == ',' || c == ';';
}

bool isAny(char c, const std::string &chars) {
  return chars.find(c) != std::string::npos;
}

bool isDigit(char c) {
  return std::isdigit((unsigned char)c);
}

bool isIdentifierStart(char c) {
  return std::isalpha((unsigned char)c) || c == '_';
}

bool isIdentifierPart(char c) {
  return isIdentifierStart(c) || isDigit(c);
}

bool isEOF(char c) {
  return c == '\0';
}

// I decided to move the implementation of of the Token class
// from Token.h to Tokenizer.cpp
//    IMPLEMENTATION: Token class
Token::Token()
  : type(Token::Eof), image() {}

Token::Token(Token::Type t, const std::string& i)
  : type(t), image(i) { }

bool Token::operator==(const std::string& other) const {
  return image == other;
}

bool Token::operator==(Token::Type other) const {
  if(type == Number) {
    return other == Number || other == PositiveNumber || 
           other == NegativeNumber || other == FloatingPointNumber;
  }
  else if(other == Number) {
    return type == Number || type == PositiveNumber || 
           type == NegativeNumber || type == FloatingPointNumber;
  }

  return type == other;
}

bool Token::operator==(const Token& other) const {
  return (*this == other.image) && (*this == other.type);
}

bool Token::operator!=(const std::string& other) const {
  return !(*this == other);
}

bool Token::operator!=(Token::Type other) const {
  return !(*this == other);
}

bool Token::operator!=(const Token& other) const {
  return !(*this == other);
}

unsigned long long Token::toUInt() const {
  return std::stoul(image);
}

long long Token::toInt() const {
  return std::stol(image);
}

double Token::toDouble() const {
  return std::stod(image);
}

Token Token::createArithmeticSign(char src) {
  return Token(ArithmeticSign, std::string(1, src));
}

Token Token::createSeparator(char src) {
  return Token(Separator, std::string(1, src));
}

Token Token::createNumber(const std::string& number, bool is_positive, bool is_float) {
  if(is_float) {
    return Token(FloatingPointNumber, number);
  }
  else if(is_positive) {
    return Token(PositiveNumber, number);
  }

  return Token(NegativeNumber, number);
}
//    END OF IMPLEMENTATION Token class

const Token&
Tokenizer::getCurrentToken() const {
  return currentToken;
}

char Tokenizer::getCurrentChar() const {
  return currentChar;
}

unsigned long long Tokenizer::charCount() const {
  return charCnt;
}

unsigned long long Tokenizer::lineCount() const {
  return lineCnt;
}

Tokenizer::Tokenizer() :
  currentChar(0), currentToken(), started(false), addLine(false),
  charCnt(0), lineCnt(0) {}

void Tokenizer::saveTokenIfNotEof(const Token& token) {
  if(token != Token::Eof)
    saveToken(token);
}

void Tokenizer::saveToken(const Token& token) {
  tokenStack.push_back(token);
}

char Tokenizer::getNextChar() {
  currentChar = doGetNextChar();
  return currentChar;
}

const Token& Tokenizer::getNextToken() {
  if (tokenStack.size() > 0) {
    Token result = tokenStack.back();
    tokenStack.pop_back();
    currentToken = result;
    return currentToken;
  }

  if (!started) {
    currentChar = getNextChar();
    started = true;
  }

  while (isSpace(currentChar))
    currentChar = getNextChar();

  if (isEOF(currentChar)) {
    currentToken = Token(Token::Eof, "");
  }
  else if (isSeparator(currentChar)) {
    currentToken = Token::createSeparator(currentChar);
    currentChar = getNextChar();
  }
  else if (currentChar == '+') {
    currentToken = Token::createArithmeticSign(currentChar);
    currentChar = getNextChar();
  }
  else if (currentChar == '-') {
    currentChar = getNextChar();
    if (!isDigit(currentChar))
      currentToken = Token::createArithmeticSign('-');
    else { // Negative number
      bool is_float;
      std::string literal = "-" + parseNumber(is_float);
      
      currentToken = Token::createNumber(literal, false, is_float);
    }
  }
  else if (currentChar == '\"') {
    std::string literal = ""; // std::string(1, currentChar);
    currentChar = getNextChar();

    while (currentChar != '\"' && !isEOF(currentChar)) {
      literal += currentChar;
      currentChar = getNextChar();
    }

    if (currentChar == '\"') {
      // literal += currentChar;
      currentChar = getNextChar();
    }
    else {
      std::cout << "Error in literal parsing: reached EOF before literal end" << std::endl;
    }

    currentToken = Token(Token::StringLiteral, literal);
  }
  else if (isDigit(currentChar)) {
    bool is_float;
    std::string literal = parseNumber(is_float);

    currentToken = Token::createNumber(literal, true, is_float);
  }
  else if (isIdentifierStart(currentChar)) {
    std::string identifier = std::string(1, currentChar);
    currentChar = getNextChar();

    while (isIdentifierPart(currentChar)) {
      identifier += currentChar;
      currentChar = getNextChar();
    }

    currentToken = Token(Token::Identifier, identifier);
  }
  else {
    std::string exceptStr = "Invalid character \"" + std::string(1, currentChar) + "\" "
                            "(ascii " + std::to_string(currentChar) + ") encountered at line " + std::to_string(lineCount());
    throw CppCAN::CANDatabaseException(exceptStr);
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

std::string Tokenizer::parseNumber(bool& is_float) {
  std::string result(1, getCurrentChar());
  
  char currentChar = getNextChar();
  is_float = false;

  while ((isDigit(currentChar) || isAny(currentChar,".e"))&& !isEOF(currentChar)) {
    result += currentChar;
    currentChar = getNextChar();

    if(currentChar == '.') {
      is_float = true;
      result += currentChar;
      currentChar = getNextChar();
    }
    else if(currentChar == 'e') {
      result += currentChar;
      currentChar = getNextChar();
      
      // Plus "in the wild" are not considered to be part of a number
      // They are only allowed after "e" (eg. 3e+002)
      if(currentChar == '+') {
        result += currentChar;
        currentChar = getNextChar();
      }
      // Negative exposants always represent floating-point numbers
      else if(currentChar == '-') {
        result += currentChar;
        is_float = true;
        currentChar = getNextChar();
      }
    }
  }

  return result;
}

void Tokenizer::skipUntil(const std::string& token) {
  unsigned long long initLine = lineCount();

  while(currentToken != token &&
        currentToken != Token::Eof) {
    getNextToken();
  }

  if(currentToken.type == Token::Eof) {
    throw CANDatabaseException(
      "Error: due to an unrecognized (and badly formed) command at line " +
      std::to_string(initLine) +
      " the parsing has failed"
    );
  }
}

FileTokenizer::FileTokenizer(const std::string& filename)
  : Tokenizer(), inputstream(filename) {

}

char FileTokenizer::doGetNextChar() {
  if (inputstream.eof())
    return 0;

  char result = 0;
  inputstream.get(result);

  charCnt += 1;
  if (addLine) {
    lineCnt += 1;
    addLine = false;
  }
  if (result == '\n') {
    addLine = true;
  }

  return result >= 0 ? result : 0;
}


StringTokenizer::StringTokenizer(const std::string& src_string)
  : Tokenizer(), src_str(src_string) {

}

char StringTokenizer::doGetNextChar() {
  if (charCnt >= src_str.size())
    return 0;

  char result = src_str[charCnt++];
  
  if (addLine) {
    lineCnt += 1;
    addLine = false;
  }

  if (result = '\n') {
    addLine = true;
  }

  return result;
}