#include "operations.h"
#include "CANDatabaseAnalysis.h"
#include "CANDatabase.h"
#include <algorithm>

void CppCAN::can_parse::check_all_frames(CANDatabase& db) {
  std::vector<uint32_t> ids;
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
