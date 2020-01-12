#include "operations.h"
#include "CANDatabase.h"
#include <iostream>
#include <iomanip>

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
  std::cout << frame.name() << ":\t"  
            << std::hex << std::showbase << frame.can_id() << "/" 
            << std::dec << std::noshowbase << frame.dlc() << "/"
            << frame.period() << "ms" << std::endl;

  if(frame.comment().size() > 0) {
    std::cout << "COMMENT" << frame.name() << ":\t \"" << frame.comment() << "\"" << std::endl;
  }
}

void print_signal_impl(const CANSignal& sig) {
  std::cout << "SIGNAL[" << sig.name() << "]: " << std::endl;
  std::cout << "\tstart bit:\t" << sig.start_bit() << std::endl;
  std::cout << "\tlength:\t\t" << sig.length() << std::endl;
  std::cout << "\tendianness:\t\t" << ((sig.endianness() == CANSignal::BigEndian) 
                                      ? "BigEndian" : "LittleEndian") << std::endl;
  std::cout << "\tsignedness:\t\t" << ((sig.signedness() == CANSignal::Signed) 
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

void CppCAN::can_parse::print_single_frame(CANDatabase& db, uint32_t can_id) {
  const CANFrame& frame = db[can_id];

  print_frame_impl(frame);
  
  print_info_separator(1);
  
  // First, explore the database to find "pretty-printing" parameters
  int sig_name_maxsize = 15; // At least a reasonable column size
  for(const auto& sig : frame) {
    if(sig.second.name().size() > sig_name_maxsize)
      sig_name_maxsize = sig.second.name().size() + 1;
  }

  std::cout << std::left << std::setw(sig_name_maxsize) << "Signal name"
            << std::setw(10) << "Start bit" 
            << std::setw(9) << "Length" 
            << std::setw(9) << "Scale" 
            << std::setw(10) << "Offset" 
            << std::setw(12) << "Signedness" 
            << std::setw(15) << "Endianness" 
            << std::setw(10) << "Range" 
            << std::endl;

  for(const auto& sig : frame) {
    const CANSignal& signal = sig.second;
    std::cout << std::left << std::setw(sig_name_maxsize) << signal.name()
              << std::setw(10) << signal.start_bit()
              << std::setw(9) << signal.length() 
              << std::setw(9) << std::setprecision(3) << signal.scale() 
              << std::setw(10) << std::setprecision(3) << signal.offset() 
              << std::setw(12) << ((signal.signedness() == CANSignal::Signed) ? "Signed" : "Unsigned")
              << std::setw(15) << ((signal.endianness() == CANSignal::BigEndian) ? "BigEndian" : "LittleEndian")
              << std::setw(10) << ("[" + std::to_string(signal.range().min) + ", " + std::to_string(signal.range().max) + "]")
              << std::endl;              
  }
  
}