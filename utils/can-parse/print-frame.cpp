#include "operations.h"
#include "CANDatabaseAnalysis.h"
#include "CANDatabase.h"

void print_info_separator(int level = 0) {
  static const char* level_0_separator = "##############";
  static const char* level_1_separator = "--------------";
  static const char* level_2_separator = "~~~~~~~~~~~~~~";
  const char** separator = nullptr;

  switch(level) {
    case 0:
    default:
      separator = &level_0_separator;
      break;

    case 1:
      separator = &level_1_separator;
      break;
  }
    std::cout << *separator << std::endl;
}

void print_frame_impl(const CANFrame& frame) {
  auto choicesStr = [](const std::map<unsigned int, std::string>& choices) {
    std::string result = "";
    for(const auto& choice: choices) {
      result += std::to_string(choice.first) + " -> \"" + choice.second + "\", ";
    }
    return result;
  };
   
  std::cout << "FRAME[" << frame.name() << "]:\t"  
            << std::hex << std::showbase << frame.can_id() << "/" 
            << std::dec << std::noshowbase << frame.dlc() << "/" << std::endl
            << frame.period() << "s" << std::endl;

  if(frame.comment().size() > 0) {
    std::cout << "FRAME[" << frame.name() << "]:\t \"" << frame.comment() << "\"" << std::endl;
  }
}

void print_signal_impl(const CANSignal& sig) {
  std::cout << "SIGNAL[" << sig.name() << "]: " << std::endl;
  std::cout << "\tstart bit:\t" << sig.start_bit() << std::endl;
  std::cout << "\tlength:\t" << sig.length() << std::endl;
  std::cout << "\tendianness:\t" << ((sig.endianness() == CANSignal::BigEndian) 
                                      ? "BigEndian" : "LittleEndian") << std::endl;
  std::cout << "\tsignedness:\t" << ((sig.signedness() == CANSignal::Signed) 
                                      ? "Signed" : "Unsigned") << std::endl;
  std::cout << "\tscale:\t" << sig.scale() << std::endl;
  std::cout << "\toffset:\t" << sig.offset() << std::endl;
  std::cout << "\trange:\t" << sig.range().min << " -> " << sig.range().max << std::endl;


  if(sig.choices().size() > 0) {
    std::cout << "\tchoices:" << std::endl;
    
    for(const auto& choice: sig.choices()) {
      std::string result = "";
      result += std::to_string(choice.first) + " -> \"" + choice.second + "\", ";
      std::cout << "\t\t" << result << std::endl;
    }
  }
}


void CppCAN::can_parse::print_all_frames(CANDatabase& db) {
  size_t i = 0;
  for(const auto& frame : db) {
    print_frame_impl(*frame.second);
    print_info_separator(1);
    
    size_t j = 0;
    for(const auto& sig : *frame.second) {
      print_signal_impl(*sig.second);

      if(j++ < frame.second->size() - 1)
        print_info_separator(2);
    }
    
    if(i++ < db.size() - 1)
      print_info_separator();
  }
}