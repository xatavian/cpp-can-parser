#include "models/CANDatabase.h"

#include <iostream>

void printFrame(std::shared_ptr<CANFrame> frame) {
  std::cout << "FRAME[" << frame->name() << "]: "
            << "ID(0x" << std::hex << frame->can_id() << std::dec << ") "
            << "DLC(" << frame->dlc() << ") "
            << "PERIOD(" << frame->period() << ")"
            << std::endl;

  for(const auto& sigw : frame->signals()) {
    std::shared_ptr<CANSignal> sig = sigw.lock();
    std::cout << "  SIGNAL[" << sig->name() << "]: "
              << "startBit(" << sig->start_bit() << ") "
              << "length(" << sig->length() << ") "
              << "endianness("
              << ((sig->endianness() == CANSignal::BigEndian) ? "BigEndian" : "LittleEndian")
              << ") "
              << "signedness("
              << ((sig->signedness() == CANSignal::Signed) ? "Signed" : "Unsigned")
              << ") "
              << "scale(" << sig->scale() << ") "
              << "offset(" << sig->offset() << ") "
              << "range(" << sig->range().min << " -> " << sig->range().max << ")"
              << std::endl;
  }
}
int main() {
  CANDatabase db = CANDatabase::fromFile("dbc/test.dbc");

  std::cout << "Exploring the CAN Database \"" << db.filename() << "\" "
            << "(size= " << db.size() << "):"
            << std::endl;

  for(const auto& framew: db.frames()) {
      printFrame(framew.lock());
  }

  return 0;
}
