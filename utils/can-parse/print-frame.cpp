#include "operations.h"
#include "CANDatabaseAnalysis.h"
#include "CANDatabase.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace CppCAN;

/*
 * Internal pretty-printing parameters
 */
static const int INT_COL_SIZE = 10;

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
  size_t frame_name_maxsize = 12; // At least a reasonable column size
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
}
