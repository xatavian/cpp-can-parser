#include "operations.h"
#include "CANDatabase.h"
#include <iostream>
#include <iomanip>


// The following helper classes have been taken from
// https://stackoverflow.com/a/14861289/8147455
template<typename charT, typename traits = std::char_traits<charT> >
class center_helper {
    std::basic_string<charT, traits> str_;
public:
    center_helper(std::basic_string<charT, traits> str) : str_(str) {}
    template<typename a, typename b>
    friend std::basic_ostream<a, b>& operator<<(std::basic_ostream<a, b>& s, const center_helper<a, b>& c);
};

template<typename charT, typename traits = std::char_traits<charT> >
center_helper<charT, traits> centered(std::basic_string<charT, traits> str) {
    return center_helper<charT, traits>(str);
}

// redeclare for std::string directly so we can support anything that implicitly converts to std::string
center_helper<std::string::value_type, std::string::traits_type> centered(const std::string& str) {
    return center_helper<std::string::value_type, std::string::traits_type>(str);
}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const center_helper<charT, traits>& c) {
    std::streamsize w = s.width();
    if (w > c.str_.length()) {
        std::streamsize left = (w + c.str_.length()) / 2;
        s.width(left);
        s << c.str_;
        s.width(w - left);
        s << "";
    } else {
        s << c.str_;
    }
    return s;
}

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
  
  /*
  for(const auto& frame : db) {
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