#ifndef CANDatabaseException_H
#define CANDatabaseException_H

#include <exception>
#include <string>

class CANDatabaseException : public std::exception {
public:
  CANDatabaseException(const std::string& description) :
   description_(description) {}

  const char* what() const noexcept {
    return description_.c_str();
  }
private:
  std::string description_;
};

#endif
