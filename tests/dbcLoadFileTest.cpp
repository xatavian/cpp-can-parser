#include "CANDatabase.h"

#include <iostream>

void printFrame(std::shared_ptr<CANFrame> frame) {
  auto choicesStr = [](const std::map<unsigned int, std::string>& choices) {
    std::string result = "";
    for(const auto& choice: choices) {
      result += std::to_string(choice.first) + " -> \"" + choice.second + "\", ";
    }
    return result;
  };
  
  std::cout << "FRAME[" << frame->name() << "]: "
            << "ID(0x" << std::hex << frame->can_id() << std::dec << ") "
            << "DLC(" << frame->dlc() << ") "
            << "PERIOD(" << frame->period() << ")"
            << std::endl;

  for(const auto& sigw : *frame) {
    std::shared_ptr<CANSignal> sig = sigw.second;
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
              << "range(" << sig->range().min << " -> " << sig->range().max << ") "
              << "choices(" << choicesStr(sig->choices()) << ") "
              << std::endl;
  }
}
int main() {
  CANDatabase db = CANDatabase::fromFile("dbc/test.dbc");

  std::cout << "Exploring the CAN Database \"" << db.filename() << "\" "
            << "(size= " << db.size() << "):"
            << std::endl;

  for(const auto& framew: db) {
      printFrame(framew.second);
  }

  return 0;
}
