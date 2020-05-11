#include "operations.h"
#include "cpp-can-parser/CANDatabaseAnalysis.h"
#include "cpp-can-parser/CANDatabase.h"
#include <algorithm>
#include <iostream>

static void
print_frame_result(const CppCAN::CANFrame& frame, const std::vector<std::string>& diagnosis) {
  std::string error_sig_names = "\"" + *diagnosis.begin() + "\"";
  std::for_each(diagnosis.begin() + 1, diagnosis.end(), [&error_sig_names](const std::string& sig_name) {
    error_sig_names += ", \"" + sig_name + "\"";
  });

  std::cout << "Layout issue(s) have been identified in frame "
            << std::hex << std::showbase << frame.can_id()
            << std::endl;
  std::cout << "The signals " << error_sig_names << " are overlapping" << std::endl;
}

bool CppCAN::can_parse::check_all_frames(CANDatabase& db, const std::vector<CANDatabase::parsing_warning>& warnings) {
  std::vector<unsigned long long> ids;

  if(warnings.size() > 0) {
    std::cout << "The following warnings have been detected during parsing: " << std::endl;
    for(const auto& w : warnings) {
      std::cout << "(Line " << w.line + 1 << ") " << w.description << std::endl;
    }
    std::cout << std::endl;
  }

  bool success = true;
  for(const auto& frame : db) {
    std::vector<std::string> diagnosis;
    if(!CppCAN::analysis::is_frame_layout_ok(frame.second, diagnosis)) {
      success = false;
      print_frame_result(frame.second, diagnosis);
    }
  }

  if(warnings.size() == 0 && success) {
    std::cout << "No layout issue have been found in the CAN database." << std::endl;
    return true;
  }

  return false;  
}

bool CppCAN::can_parse::check_single_frame(CANDatabase& db, uint32_t can_id, 
                                           const std::vector<CANDatabase::parsing_warning>& warnings) {
  if(warnings.size() > 0) {
    std::cout << "The following warnings have been detected during parsing: " << std::endl;
    for(const auto& w : warnings) {
      std::cout << "(Line " << w.line + 1 << ") " << w.description << std::endl;
    }
    std::cout << std::endl;
  }

  if(!db.contains(can_id)) {
    std::cout << "Error: The database does not contain any CAN frame " 
              << std::hex << std::showbase << can_id << std::endl;
    return false;
  }

  std::vector<std::string> diagnosis;
  if(CppCAN::analysis::is_frame_layout_ok(db.at(can_id), diagnosis)) {
    std::cout << "No layout issue for frame "
              << std::hex << std::showbase << can_id
              << std::endl;
    return true;
  }
  else {
    print_frame_result(db.at(can_id), diagnosis);
    return false;
  }
}