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

  unsigned long long toUInt() const {
    return std::stoul(image_);
  }

  long long toInt() const {
    return std::stol(image_);
  }

  double toDouble() const {
    return std::stod(image_);
  }
  
private:
  Type type_;
  std::string image_;
};

#endif
