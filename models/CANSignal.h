#ifndef CANSignal_H
#define CANSignal_H

#include <string>
#include <memory>
#include <iostream>

class CANSignal {
public:
  struct Range {
    static Range fromString(const std::string& minstr, const std::string& maxstr) {
      long min = std::stol(minstr);
      long max = std::stol(maxstr);

      return Range(min, max);
    }
    Range() :
      defined(false), min(0), max(0) {}
    Range(long m, long mm) :
      defined(true), min(m), max(mm) {}

    bool defined;
    long min;
    long max;
  };

  enum Signedness {
    Unsigned, Signed
  };

  enum Endianness {
    BigEndian, LittleEndian
  };
public:
  CANSignal(const std::string& name, unsigned int start_bit, unsigned int length,
            double scale, double offset, Signedness signedness, Endianness endianness, Range range = Range()) :
    name_(name), start_bit_(start_bit), length_(length),
    scale_(scale), offset_(offset), signedness_(signedness), endianness_(endianness),
    range_(range) { }

  const std::string& name() const {
    return name_;
  }

  unsigned int start_bit() const {
    return start_bit_;
  }

  unsigned int length() const {
    return length_;
  }

  const std::string& comment() const {
    return comment_;
  }

  int scale() const {
    return scale_;
  }

  int offset() const {
    return offset_;
  }

  const Range& range() const {
    return range_;
  }

  Signedness signedness() const {
    return signedness_;
  }

  Endianness endianness() const {
    return endianness_;
  }

  void setComment(const std::string& comment) {
    comment_ = comment;
  }
private:
  std::string name_;
  unsigned int start_bit_;
  unsigned int length_;
  double scale_;
  double offset_;
  Signedness signedness_;
  Endianness endianness_;
  Range range_;
  std::string comment_;
};

#endif
