#ifndef DBCParser_H
#define DBCParser_H

#include "CANDatabase.h"
#include "Tokenizer.h"
#include <set>
#include <memory>

namespace DBCParser {
  CANDatabase fromTokenizer(const std::string& name,
                            Tokenizer& tokenizer);

  CANDatabase fromTokenizer(Tokenizer& tokenizer);
};

#endif
