#include "operations.h"
#include "CANDatabaseAnalysis.h"
#include "CANDatabase.h"
#include <algorithm>

bool CppCAN::can_parse::check_all_frames(CANDatabase& db, const std::vector<CANDatabase::parsing_warning>& warnings) {
  std::vector<uint32_t> ids;

  if(warnings.size() > 0) {
    std::cout << "The following warnings have been detected during parsing: " << std::endl;
    for(const auto& w : warnings) {
      std::cout << "(Line " << w.line + 1 << ") " << w.description << std::endl;
    }
    std::cout << std::endl;
  }


  for(const auto& frame : db) {
    if(!CppCAN::analysis::is_frame_layout_ok(frame.second))
      ids.push_back(frame.second.can_id());
  }

  if(ids.size() == 0)
    std::cout << "No layout issue have been found in the CAN database." << std::endl;
  else {
    std::cout << "Some layout issues have been found in the database for the following frames: ";
    std::cout << std::hex << std::showbase;

    for(size_t i = 0; i < ids.size() - 1; i++) {
      std::cout << ids[i] << ", ";
    }
    std::cout << ids.back() << std::endl;
  }

  if(warnings.size() > 0 || ids.size() > 0)
    return false;
  
  return true;
}
