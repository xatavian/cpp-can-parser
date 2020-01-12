#include "operations.h"
#include "CANDatabaseAnalysis.h"
#include "CANDatabase.h"
#include <algorithm>
#include <iomanip>

/*
 * Internal pretty-printing parameters
 */
static const int INT_COL_SIZE = 10;

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

void print_frame_impl(const CANFrame& frame, unsigned name_col_size) {   
  std::cout << std::left << std::setw(name_col_size) << frame.name() << ":\t"  
            << std::hex << std::showbase << frame.can_id() << "/" 
            << std::dec << std::noshowbase << frame.dlc() << "/"
            << frame.period() << "s" << std::endl;

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

void print_frame_line(const CANFrame& src, int name_col_size) {
  std::cout << std::left
            << std::setw(name_col_size) << src.name() 
            << std::setw(INT_COL_SIZE)  << std::hex << std::showbase << src.can_id() 
            << std::setw(INT_COL_SIZE)  << std::dec << src.dlc()
            << std::setw(INT_COL_SIZE)  << std::to_string(src.period()) + " s"
            << std::endl;
}

void CppCAN::can_parse::print_all_frames(CANDatabase& db) {
  size_t i = 0;

  // First, explore the database to find "pretty-printing" parameters
  int frame_name_maxsize = 12; // At least a reasonable column size
  for(const auto& frame : db) {
    if(frame.second.name().size() > frame_name_maxsize)
      frame_name_maxsize = frame.second.name().size() + 1;
  }

  int full_line_size = frame_name_maxsize + INT_COL_SIZE * 3; 
  std::cout << std::left;
  std::cout << std::setw(frame_name_maxsize) << "Frame name"
            << std::setw(INT_COL_SIZE) << "CAN ID" 
            << std::setw(INT_COL_SIZE) << "DLC"
            << std::setw(INT_COL_SIZE) << "Period"
            << std::endl;
  std::cout << std::setfill('-') << std::setw(full_line_size) << "" << std::endl;
  std::cout.fill(' ');
             
  for(const auto& frame : db) {
    print_frame_line(frame.second, frame_name_maxsize);
  }
  
  /*
  for(const auto& frame : db) {
    print_frame_impl(*frame.second, frame_name_maxsize);
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
  */
}