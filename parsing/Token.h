#ifndef Token_H
#define Token_H

#include <string>

class Token {
public:
  enum Type {
      Literal,
      Separator,
      Number,
      Identifier,
      Sign,
      Eof
  };

  Token() = default;
  Token(Type type, const std::string& image) :
   type_(type), image_(image) {}
  Token(const Token&) = default;
  Token& operator=(const Token&) = default;

  Type type() const {
    return type_;
  }
  const std::string& image() const {
    return image_;
  }

private:
  Type type_;
  std::string image_;
};

#endif
