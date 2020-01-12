#include "CANDatabase.h"
#include "DBCParser.h"
#include <utility>
#include <iostream>

CANDatabase::CANDatabase(const std::string& filename)
  : filename_(filename) { }

const std::string& CANDatabase::filename() const {
  return filename_;
}

std::size_t CANDatabase::size() const {
  return map_.size();
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

const CANFrame& CANDatabase::at(const std::string& name) const {
  const IDKey& map_key = strKeyIndex_.at(name);
  return map_.at(map_key);
}

CANFrame& CANDatabase::at(const std::string& name) {
  const IDKey& map_key = strKeyIndex_.at(name);
  return map_.at(map_key);
}

const CANFrame& CANDatabase::at(unsigned long long id) const {
  const IDKey& map_key = intKeyIndex_.at(id);
  return map_.at(map_key);
}

CANFrame& CANDatabase::at(unsigned long long id) {
  const IDKey& map_key = intKeyIndex_.at(id);
  return map_.at(map_key);
}

void CANDatabase::addFrame(const CANFrame& frame) {
  if(strKeyIndex_.find(frame.name()) != strKeyIndex_.end()) {
    std::cout << "WARNING: Double declaration of a frame with name "
              << "\"" << frame.name() << "\"" << std::endl;
  }
  
  if(intKeyIndex_.find(frame.can_id()) != intKeyIndex_.end()) {
    std::cout << "WARNING: Double declaration of a frame with id "
              << frame.can_id() << std::endl;
  }

  IDKey map_key = { frame.name(), frame.can_id() };

  map_.insert(std::make_pair(map_key, frame));
  strKeyIndex_.insert(std::make_pair(frame.name(), map_key));
  intKeyIndex_.insert(std::make_pair(frame.can_id(), map_key));
}

void CANDatabase::removeFrame(const std::string& name) {
  try {
    const IDKey& map_key = strKeyIndex_.at(name);

    map_.erase(map_.find(map_key));
    strKeyIndex_.erase(strKeyIndex_.find(map_key.str_key));
    intKeyIndex_.erase(intKeyIndex_.find(map_key.int_key));
  } 
  catch(const std::out_of_range&) {
    std::string excepText = "Cannot remove frame with name " + name;
    throw std::out_of_range(excepText);
  }
}

void CANDatabase::removeFrame(unsigned int can_id) {
  try {
    const IDKey& map_key = intKeyIndex_.at(can_id);

    map_.erase(map_.find(map_key));
    strKeyIndex_.erase(strKeyIndex_.find(map_key.str_key));
    intKeyIndex_.erase(intKeyIndex_.find(map_key.int_key));
  } 
  catch(const std::out_of_range&) {
    std::string excepText = "Cannot remove frame with CAN ID ";
    excepText += std::to_string(can_id);
    throw std::out_of_range(excepText);
  }
}

bool CANDatabase::contains(unsigned long long can_id) const {
  return intKeyIndex_.find(can_id) != intKeyIndex_.end();
}

bool CANDatabase::contains(const std::string& name) const {
  return strKeyIndex_.find(name) != strKeyIndex_.end();
}

CANDatabase::iterator 
CANDatabase::begin() {
  return map_.begin();
}

CANDatabase::const_iterator 
CANDatabase::begin() const {
  return map_.begin();
}

CANDatabase::const_iterator
CANDatabase::cbegin() const {
  return map_.cbegin();
}

CANDatabase::iterator 
CANDatabase::end() {
  return map_.end();
}

CANDatabase::const_iterator 
CANDatabase::end() const {
  return map_.end();
}

CANDatabase::const_iterator
CANDatabase::cend() const {
  return map_.cend();
}

CANDatabase::reverse_iterator 
CANDatabase::rbegin() {
  return map_.rbegin();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rbegin() const {
  return map_.rbegin();
}

CANDatabase::const_reverse_iterator
CANDatabase::crbegin() const {
  return map_.crbegin();
}

CANDatabase::reverse_iterator 
CANDatabase::rend() {
  return map_.rend();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rend() const {
  return map_.rend();
}

CANDatabase::const_reverse_iterator
CANDatabase::crend() const {
  return map_.crend();
}

void CANDatabase::clear() {
  map_.clear();
  intKeyIndex_.clear();
  strKeyIndex_.clear();
}

void swap(CANDatabase & first, CANDatabase & second) {
  std::swap(first.intKeyIndex_, second.intKeyIndex_);
  std::swap(first.strKeyIndex_, second.strKeyIndex_);
  std::swap(first.map_, second.map_);
  std::swap(first.filename_, second.filename_);
}

const CANFrame& CANDatabase::operator[](unsigned long long can_id) const {
  const IDKey& map_key = intKeyIndex_.at(can_id);
  return map_.at(map_key);
}

CANFrame& CANDatabase::operator[](unsigned long long can_id) {
  const IDKey& map_key = intKeyIndex_.at(can_id);
  return map_.at(map_key);
}

const CANFrame& CANDatabase::operator[](const std::string& name) const {
  const IDKey& map_key = strKeyIndex_.at(name);
  return map_.at(map_key);
}

CANFrame& CANDatabase::operator[](const std::string& name) {
  const IDKey& map_key = strKeyIndex_.at(name);
  return map_.at(map_key);
}

bool CANDatabase::IntIDKeyCompare::operator()(const IDKey& k1, const IDKey& k2) const {
  return k1.int_key < k2.int_key;
}
