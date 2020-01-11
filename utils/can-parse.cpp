#include "CANDatabase.h"
#include "CANDatabaseAnalysis.h"
#include <iostream>
#include <cstring>
#include <algorithm>

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

void executeAction(CANDatabase& db, int argc, char** argv) {
  if(strcmp(argv[1], "checkframe") == 0) {
    std::vector<uint64_t> ids;
    for(const auto& frame : db) {
      if(!CppCAN::analysis::is_frame_layout_ok(*frame.second))
        ids.push_back(frame.second->can_id());
    }

    if(ids.size() == 0)
      std::cout << "No layout issue have been found in the CAN database." << std::endl;
    else {
      std::cout << "Some layout have been found in the database for the following frames: ";
      std::cout << std::hex << std::showbase;
      std::for_each(ids.begin(), ids.end(), [](uint64_t id) { std::cout << id << ", "; });
    }
  }
  else {
    for (const auto& framew : db) {
      printFrame(framew.second);
    }
  }
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
    executeAction(db, argc, argv);
  }
  catch (const CANDatabaseException& e) {
    std::cerr << "An error happened while parsing the database: " 
              << e.what() << std::endl;
    return 1;
  }

  return 0;
}
