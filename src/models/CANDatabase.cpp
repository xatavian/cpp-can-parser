#include "CANDatabase.h"
#include "DBCParser.h"
#include <utility>
#include <iostream>

CANDatabase::CANDatabase(const std::string& filename)
  : filename_(filename), strIndex_(), intIndex_() { }

CANDatabase::CANDatabase(const CANDatabase& src):
  filename_(src.filename_), strIndex_(), intIndex_() {

  for (const auto& frame : src) {
    auto copy = std::make_shared<CANFrame>(*frame.second);
    intIndex_.insert(std::make_pair(frame.second->can_id(), copy));
    strIndex_.insert(std::make_pair(frame.second->name(), copy));
  }
}

CANDatabase & CANDatabase::operator=(CANDatabase src) {
  swap(*this, src);
  return *this;
}

CANDatabase::CANDatabase(CANDatabase && src)
  : filename_(), strIndex_(), intIndex_() {

  swap(*this, src);
}

CANDatabase & CANDatabase::operator=(CANDatabase && src) {
  swap(*this, src);
  return *this;
}

const std::string& CANDatabase::filename() const {
  return filename_;
}

std::size_t CANDatabase::size() const {
  return intIndex_.size();
}

CANDatabase CANDatabase::fromFile(const std::string& filename) {
  std::ifstream test_stream(filename);
  if (!test_stream.good()) {
    throw CANDatabaseException("Cannot find file " + filename);
  }
  
  FileTokenizer tokenizer(filename);
  return DBCParser::fromTokenizer(filename, tokenizer);
}

CANDatabase CANDatabase::fromString(const std::string & src_string) {
  StringTokenizer tokenizer(src_string);
  return DBCParser::fromTokenizer(tokenizer);
}

std::shared_ptr<CANFrame> CANDatabase::at(const std::string& name) const {
  return strIndex_.at(name);
}

std::shared_ptr<CANFrame> CANDatabase::at(unsigned long long id) const {
  return intIndex_.at(id);
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
    std::string excepText = "Cannot remove frame with name " + name;
    throw std::out_of_range(excepText);
  }

  unsigned long long intIdx = ite->second->can_id();
  strIndex_.erase(ite); // No need for a second lookup
  intIndex_.erase(intIndex_.find(intIdx));
}

void CANDatabase::removeFrame(unsigned int can_id) {
  auto ite = intIndex_.find(can_id);
  if(ite == intIndex_.end()) {
    std::string excepText = "Cannot remove frame with CAN ID " + std::to_string(can_id); 
    throw std::out_of_range(excepText); 
  }

  std::string strIdx = ite->second->name();
  intIndex_.erase(ite); // No need for a second lookup
  strIndex_.erase(strIndex_.find(strIdx));
}

bool CANDatabase::contains(unsigned long long can_id) const {
  return intIndex_.find(can_id) != intIndex_.end();
}

bool CANDatabase::contains(const std::string& name) const {
  return strIndex_.find(name) != strIndex_.end();
}

CANDatabase::iterator 
CANDatabase::begin() {
  return intIndex_.begin();
}

CANDatabase::const_iterator 
CANDatabase::begin() const {
  return intIndex_.begin();
}

CANDatabase::const_iterator
CANDatabase::cbegin() const {
  return intIndex_.cbegin();
}

CANDatabase::iterator 
CANDatabase::end() {
  return intIndex_.end();
}

CANDatabase::const_iterator 
CANDatabase::end() const {
  return intIndex_.end();
}

CANDatabase::const_iterator
CANDatabase::cend() const {
  return intIndex_.cend();
}

CANDatabase::reverse_iterator 
CANDatabase::rbegin() {
  return intIndex_.rbegin();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rbegin() const {
  return intIndex_.rbegin();
}

CANDatabase::const_reverse_iterator
CANDatabase::crbegin() const {
  return intIndex_.crbegin();
}

CANDatabase::reverse_iterator 
CANDatabase::rend() {
  return intIndex_.rend();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rend() const {
  return intIndex_.rend();
}

CANDatabase::const_reverse_iterator
CANDatabase::crend() const {
  return intIndex_.crend();
}

void CANDatabase::clear() {
  intIndex_.clear();
  strIndex_.clear();
}

void swap(CANDatabase & first, CANDatabase & second) {
  std::swap(first.intIndex_, second.intIndex_);
  std::swap(first.strIndex_, second.strIndex_);
  std::swap(first.filename_, second.filename_);
}

std::shared_ptr<CANFrame> CANDatabase::operator[](unsigned long long can_id) const {
  auto ite = intIndex_.find(can_id);
  if (ite == intIndex_.end())
    return std::shared_ptr<CANFrame>();
  return ite->second;
}

std::shared_ptr<CANFrame> CANDatabase::operator[](const std::string& name) const {
  auto ite = strIndex_.find(name);
  if (ite == strIndex_.end())
    return std::shared_ptr<CANFrame>();
  return ite->second;
}