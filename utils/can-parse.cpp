#include "CANDatabase.h"

#include <iostream>
#include <windows.h>

void printFrame(std::shared_ptr<CANFrame> frame) {
  auto choicesStr = [](const std::map<unsigned int, std::string>& choices) {
    std::string result = "";
    for(const auto& choice: choices) {
      result += std::to_string(choice.first) + " -> \"" + choice.second + "\", ";
    }
    return result;
  };
  
  std::cout << "FRAME[" << frame->name() << "]:" << std::endl 
            << "- ID(0x" << std::hex << frame->can_id() << std::dec << ")"  << std::endl
            << "- DLC(" << frame->dlc() << ")" << std::endl
            << "- PERIOD(" << frame->period() << ")" << std::endl
            << "- COMMENT(" << frame->comment() << ")" << std::endl;

  for(const auto& sigw : *frame) {
    std::shared_ptr<CANSignal> sig = sigw.second;
    std::cout << "SIGNAL[" << sig->name() << "]: "
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

void showUsage(char* program_name) {
  std::cerr << "Usage: " << program_name << " <path/to/file>" << std::endl;
  std::cerr << "Currently supported formats: DBC" << std::endl;
}
std::string ExePath() {
  char buffer[MAX_PATH];
  GetModuleFileName(NULL, buffer, MAX_PATH);
  std::string::size_type pos = std::string(buffer).find_last_of("\\/");
  return std::string(buffer).substr(0, pos);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    showUsage(argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "-h") == 0) {
    showUsage(argv[0]);
    return 0;
  }

  
  try {
    CANDatabase db = CANDatabase::fromFile(argv[1]);

    std::cout << "Exploring the CAN Database \"" << db.filename() << "\" "
      << "(size= " << db.size() << "):"
      << std::endl;

    for (const auto& framew : db) {
      printFrame(framew.second);
    }
  }
  catch (const CANDatabaseException& e) {
    std::cerr << "An error happened while parsing the database: " 
              << e.what() << std::endl;
    std::cerr << "CWD: " << ExePath() << std::endl;
    return 1;
  }

  return 0;
}
