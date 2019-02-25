#include "CANDatabase.h"
#include "parsing/DBCParser.h"
#include <utility>
#include <iostream>

CANDatabase::CANDatabase(const std::string& filename)
  : filename_(filename), strIndex_(), intIndex_() { }

const std::string& CANDatabase::filename() const {
  return filename_;
}

std::size_t CANDatabase::size() const {
  return intIndex_.size();
}

CANDatabase CANDatabase::fromFile(const std::string& filename) {
  Tokenizer tokenizer(filename);

  if(!tokenizer.is_valid()) {
    throw CANDatabaseException("Cannot find file " + filename);
  }

  return DBCParser::fromTokenizer(filename, tokenizer);
}

std::weak_ptr<CANFrame> CANDatabase::getFrameByName(const std::string& name) const {
  std::weak_ptr<CANFrame> result;

  auto ite = strIndex_.find(name);
  if(ite != strIndex_.end())
    result = ite->second;

  return result;
}

std::weak_ptr<CANFrame> CANDatabase::getFrameById(unsigned int id) const {
  std::weak_ptr<CANFrame> result;

  auto ite = intIndex_.find(id);
  if(ite != intIndex_.end())
    result = ite->second;

  return result;
}

std::vector<std::weak_ptr<CANFrame>> CANDatabase::frames() const {
  std::vector<std::weak_ptr<CANFrame>> result;
  result.reserve(intIndex_.size());

  for(const auto& sigIte: intIndex_) {
    result.emplace_back(sigIte.second);
  }
  return result;
}

void CANDatabase::addFrame(std::shared_ptr<CANFrame> frame) {
  if(strIndex_.find(frame->name()) != strIndex_.end()) {
    std::cout << "WARNING: Double declaration of a frame with name "
              << "\"" << frame->name() << "\"" << std::endl;
  }
  if(intIndex_.find(frame->can_id()) != intIndex_.end()) {
    std::cout << "WARNING: Double declaration of a frame with id "
              << frame->can_id() << std::endl;
  }

  strIndex_.insert(std::make_pair(frame->name(), frame));
  intIndex_.insert(std::make_pair(frame->can_id(), frame));
}

void CANDatabase::removeFrame(const std::string& name) {
  auto ite = strIndex_.find(name);
  if(ite == strIndex_.end()) {
    std::cout << "WARNING: Cannot remove frame with name \""
              << name << "\"" << std::endl;
    return;
  }

  unsigned int intIdx = ite->second->can_id();
  strIndex_.erase(ite); // No need for a second lookup
  intIndex_.erase(intIndex_.find(intIdx));
}

void CANDatabase::removeFrame(unsigned int can_id) {
  auto ite = intIndex_.find(can_id);
  if(ite == intIndex_.end()) {
    std::cout << "WARNING: Cannot remove frame with id "
              << can_id << std::endl;
    return;
  }

  std::string strIdx = ite->second->name();
  intIndex_.erase(ite); // No need for a second lookup
  strIndex_.erase(strIndex_.find(strIdx));
}
