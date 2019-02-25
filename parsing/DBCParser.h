#ifndef DBCParser_H
#define DBCParser_H

#include "models/CANDatabase.h"
#include "parsing/Tokenizer.h"
#include <set>
#include <memory>

namespace DBCParser {
  CANDatabase fromTokenizer(const std::string& name,
                            Tokenizer& tokenizer);
};

#endif
