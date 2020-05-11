#ifndef Tokenizer_H
#define Tokenizer_H

#include <fstream>
#include <string>
#include <vector>
#include "Token.h"

class Tokenizer {
public:
  Tokenizer();
  virtual ~Tokenizer() = default;
  
  const Token& getNextToken();
  const Token& getCurrentToken() const;
  
  void skipLine();
  void skipUntil(const std::string& token);
  void saveToken(const Token& token);
  void saveTokenIfNotEof(const Token& token);

  unsigned long long charCount() const;
  unsigned long long lineCount() const;

protected:
  char getNextChar();
  char getCurrentChar() const;
  
  std::string parseNumber(bool& is_float);

private:
  virtual char doGetNextChar() = 0;

private:
  char currentChar;
  Token currentToken;
  std::vector<Token> tokenStack;
  bool started;

protected:
  size_t charCnt;
  size_t lineCnt;
  bool addLine;
};

class FileTokenizer : public Tokenizer {
public:
  FileTokenizer(const std::string& filename);

  virtual char doGetNextChar();

private:
  std::ifstream inputstream;

};

class StringTokenizer : public Tokenizer {
public:
  StringTokenizer(const std::string& src_string);

  virtual char doGetNextChar();

private:
  std::string src_str;
};
#endif
