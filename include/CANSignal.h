#ifndef CANSignal_H
#define CANSignal_H

#include <string>
#include <map>
#include <iostream>

class CANSignal {
public:
  struct Range {
    static Range fromString(const std::string& minstr, const std::string& maxstr);

    Range() = default; 
    Range(long m, long mm);

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
  CANSignal() = delete;
  CANSignal(const std::string& name, unsigned int start_bit, unsigned int length,
            double scale, double offset, Signedness signedness, Endianness endianness, Range range = Range());
  CANSignal(const CANSignal&) = default;
  CANSignal(CANSignal&&) = default;
  CANSignal& operator=(const CANSignal&) = default;
  CANSignal& operator=(CANSignal&&) = default;

  const std::string& name() const;

  unsigned int start_bit() const;

  unsigned int length() const;

  const std::string& comment() const;

  double scale() const;

  double offset() const;

  const Range& range() const;

  Signedness signedness() const;

  Endianness endianness() const;

  const std::map<unsigned int, std::string>& choices() const;

  void setComment(const std::string& comment);

  void setChoices(const std::map<unsigned int, std::string>& choices);

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
  std::map<unsigned int, std::string> choices_;
};

#endif
