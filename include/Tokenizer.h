#ifndef Tokenizer_H
#define Tokenizer_H

#include <fstream>
#include <string>
#include <vector>
#include "Token.h"

class Tokenizer {
public:
  Tokenizer(const std::string& filename) :
    inputstream(filename), currentChar(0), currentToken(), started(false), eofReached(false), addLine(false),
    charCnt(0), lineCnt(0) {}

  bool is_valid() const {
    return inputstream.is_open();
  }

  Token getNextToken();
  Token getCurrentToken() const;
  void skipLine();
  void skipUntil(const std::string& token);
  void saveToken(const Token& token);

  unsigned long long charCount() const;
  unsigned long long lineCount() const;

private:
  char getNextChar();

private:
  bool isSeparator(char c) const;
  bool isEOF(char c) const;
  bool isDigit(char c) const;
  bool isSpace(char c) const;
  bool isIdentifierStart(char c) const;
  bool isIdentifierPart(char c) const;

private:
  std::ifstream inputstream;
  char currentChar;
  Token currentToken;
  std::vector<Token> tokenStack;
  bool started;
  bool eofReached;
  bool addLine;
  unsigned long long charCnt;
  unsigned long long lineCnt;
};

#endif
