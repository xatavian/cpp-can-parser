#include "CANDatabase.h"
#include "CANDatabaseAnalysis.h"
#include <iostream>
#include <cstring>
#include <algorithm>

#include "operations.h"


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
  ostrm << "Usage: " << program_name << " [ACTION] <path/to/file>" << std::endl;
  ostrm << "When no action is specified, defaults to " << PRINTFRAME_ACTION << std::endl;
  ostrm << "Possible actions: " << std::endl;
  ostrm << "\t" << PRINTFRAME_ACTION << "\tPrint the content of the CAN database" << std::endl;
  ostrm << "\t" << CHECKFRAME_ACTION << "\tCheck different properties of the CAN database" << std::endl;
  ostrm << "Currently supported formats: DBC" << std::endl;
}


std::tuple<CanParseAction, std::string> extractAction(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc); // +1 so we ignore the executable name
  
  CanParseAction action = None;
  std::string src_file;

  if (args.size() < 1) {
    throw CppCAN::can_parse::CanParseException("Not enough arguments");
  }

  size_t i = 0;
  for(const std::string& arg : args) {
    // First argument, we check for potential actions
    if(i++ == 0) {
      if(arg == CHECKFRAME_ACTION) {
        action = CheckAll;
        continue;
      }
      else if(arg == PRINTFRAME_ACTION) {
        action = PrintAll;
        continue;
      }
    }

    // Check for options
    if(arg.size() > 0 && arg[0] == '-') {
      if(arg == "-h" || arg == "--help") {
        action = Help;
      }
    }
    // This is a file path (for now)
    else {
      src_file = arg;
    }
  }

  if(action != Help && src_file.size() == 0)
    throw CppCAN::can_parse::CanParseException("No source file specified");

  return std::make_tuple(action != None ? action : PrintAll, src_file);
}

int main(int argc, char** argv) {
  std::string src_file;
  CanParseAction action;

  try {
    std::tie(action, src_file) = extractAction(argc, argv);
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
      std::cerr << "Acton not implemented yet." << std::endl;
      return 3;
  }
  return 0;
}
