#include "CANDatabase.h"
#include "CANDatabaseAnalysis.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <set>
#include "operations.h"
#include <iomanip>

using namespace CppCAN::can_parse;

enum CanParseAction {
  None,
  PrintAll,
  PrintOne,
  CheckAll,
  CheckOne,
  Help
};

static std::string CHECKFRAME_ACTION = "checkframe";
static std::string PRINTFRAME_ACTION = "printframe";

void showUsage(std::ostream& ostrm, char* program_name) {
  ostrm << "Usage: " << program_name << " [ACTION [ARGUMENTS]] <path/to/file>" << std::endl;
  ostrm << "When no action is specified, defaults to " << PRINTFRAME_ACTION << std::endl;
  ostrm << "Possible actions: " << std::endl;
  ostrm << "\t" << std::left << std::setw(22) << (PRINTFRAME_ACTION + " [CAN ID]") << "Print the content of the CAN database" << std::endl;
  ostrm << "\t"              << std::setw(22) << ""                                << "if CAN ID is specified, prints the details of the given frame" << std::endl;
  ostrm << "\t"              << std::setw(22) << (CHECKFRAME_ACTION + " [CAN ID]") << "Check different properties of the CAN database" << std::endl;
  ostrm << "\t"              << std::setw(22) << ""                                << "if CAN ID is specified, print the check details of the given frame" << std::endl;
  ostrm << "Currently supported formats: DBC" << std::endl;
}


std::tuple<CanParseAction, std::string, uint32_t> extractAction(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc); // +1 so we ignore the executable name
  
  CanParseAction action = None;
  std::string src_file;
  uint32_t detail_frame = 0;

  if (args.size() < 1) {
    throw CppCAN::can_parse::CanParseException("Not enough arguments");
  }

  std::set<std::string> options;

  bool check_action = true;
  bool check_args = false;
  for(const std::string& arg : args) {
    // First argument, we check for potential actions
    if(arg.size() > 0 && arg[0] == '-') {
      options.insert(arg);
      continue;
    }
    else if(check_action) {
      if(arg == CHECKFRAME_ACTION) {
        action = CheckAll;
        check_args = true;
        check_action = false;
        continue;
      }
      else if(arg == PRINTFRAME_ACTION) {
        action = PrintAll;
        check_args = true;
        check_action = false;
        continue;
      }
      else {
        action = PrintAll;
        check_action = false;
      }
    }
    else if(check_args) {
      check_args = false;
      
      try {
        detail_frame = std::stoul(arg);
        action = static_cast<CanParseAction>(static_cast<int>(action) + 1);
        continue;
      } catch(const std::logic_error& e) {
        // The argument is not related ....
        // Probably a file name, we stick to PrintAll/CheckAll
      }
    }

    // At this point, nothing has matched. The only valid thing that this could be
    // is the file name of the database to parse.
    src_file = arg;
  }

  if(options.count("-h") > 0 || options.count("--help")) {
    action = Help;
  }

  if(action != Help && src_file.size() == 0)
    throw CppCAN::can_parse::CanParseException("No source file specified");

  return std::make_tuple(action != None ? action : PrintAll, src_file, detail_frame);
}

int main(int argc, char** argv) {
  std::string src_file;
  CanParseAction action;
  uint32_t detail_frame;

  try {
    std::tie(action, src_file, detail_frame) = extractAction(argc, argv);
  }
  catch(const CanParseException& e) {
    std::cerr << "Invalid use of the program: " << e.what() << std::endl;
    showUsage(std::cerr, argv[0]);
    return 1;
  }

  if(action == Help) {
    showUsage(std::cout, argv[0]);
    return 0;
  }

  CANDatabase db;

  try {
     db = std::move(CANDatabase::fromFile(src_file));
  }
  catch (const CANDatabaseException& e) {
    std::cerr << "An error happened while parsing the database: " 
              << e.what() << std::endl;
    return 2;
  }
  switch(action) {
    case PrintAll:
      print_all_frames(db);
      break;

    case CheckAll:
      check_all_frames(db);
      break;

    case Help:
      // Already handled before.
      break;
    
    default:
      std::cerr << "Action not implemented yet." << std::endl;
      return 3;
  }
  return 0;
}
