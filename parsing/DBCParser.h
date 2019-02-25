#ifndef DBCParser_H
#define DBCParser_H

#include "models/CANDatabase.h"
#include "parsing/Tokenizer.h"
#include <set>
#include <memory>

namespace DBCParser {
  CANDatabase fromTokenizer(const std::string& name,
                            Tokenizer& tokenizer);

  std::shared_ptr<CANSignal> parseSignal(Tokenizer& tokenizer);
  std::shared_ptr<CANFrame>  parseFrame(Tokenizer& tokenizer);
  std::set<std::string> parseECUs(Tokenizer& tokenizer);
};

#endif
