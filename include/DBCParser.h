#ifndef DBCParser_H
#define DBCParser_H

#include "CANDatabase.h"
#include "Tokenizer.h"
#include <set>
#include <memory>

namespace DBCParser {
  CANDatabase fromTokenizer(
    const std::string& name, Tokenizer& tokenizer,
    std::vector<CANDatabase::parsing_warning>* warnings = nullptr);

  CANDatabase fromTokenizer(
    Tokenizer& tokenizer, 
    std::vector<CANDatabase::parsing_warning>* warnings = nullptr);
};

#endif
