#include "cpp-can-parser/CANDatabase.h"

using namespace CppCAN;

CANSignal::Range CANSignal::Range::fromString(const std::string & minstr, const std::string & maxstr) {
  long min = std::stol(minstr);
  long max = std::stol(maxstr);

  return Range(min, max);
}

CANSignal::Range::Range(long m, long mm)
  : defined(true), min(m), max(mm) {

}

CANSignal::CANSignal(const std::string & name, unsigned int start_bit, unsigned int length, double scale, double offset, Signedness signedness, Endianness endianness, Range range) :
  name_(name), start_bit_(start_bit), length_(length),
  scale_(scale), offset_(offset), signedness_(signedness), endianness_(endianness),
  range_(range) { }

const std::string & CANSignal::name() const {
  return name_;
}

unsigned int CANSignal::start_bit() const {
  return start_bit_;
}

unsigned int CANSignal::length() const {
  return length_;
}

const std::string & CANSignal::comment() const {
  return comment_;
}

double CANSignal::scale() const {
  return scale_;
}

double CANSignal::offset() const {
  return offset_;
}

const CANSignal::Range & CANSignal::range() const {
  return range_;
}

CANSignal::Signedness CANSignal::signedness() const {
  return signedness_;
}

CANSignal::Endianness CANSignal::endianness() const {
  return endianness_;
}

const std::map<unsigned int, std::string>& CANSignal::choices() const {
  return choices_;
}

void CANSignal::setComment(const std::string & comment) {
  comment_ = comment;
}

void CANSignal::setChoices(const std::map<unsigned int, std::string>& choices) {
  choices_ = choices;
}
