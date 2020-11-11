#ifndef DBCParser_H
#define DBCParser_H

#include "cpp-can-parser/CANDatabase.h"
#include "Tokenizer.h"
#include <set>
#include <memory>

namespace CppCAN {
namespace parser {
namespace dbc {

CANDatabase fromTokenizer(
  const std::string& name, details::Tokenizer& tokenizer,
  std::vector<CANDatabase::parsing_warning>* warnings = nullptr);

CANDatabase fromTokenizer(
  details::Tokenizer& tokenizer, 
  std::vector<CANDatabase::parsing_warning>* warnings = nullptr);
  
}
}
}

#endif
