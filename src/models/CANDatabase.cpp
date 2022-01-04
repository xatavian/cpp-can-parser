#include "cpp-can-parser/CANDatabase.h"
#include "DBCParser.h"
#include <utility>
#include <iostream>

using namespace CppCAN;
namespace dtl = CppCAN::parser::details;

class CANDatabase::CANDatabaseImpl {
public:
  using container_type = CANDatabase::container_type;

  CANDatabaseImpl() =  default;
  CANDatabaseImpl(const CANDatabaseImpl&) = delete;
  CANDatabaseImpl& operator=(const CANDatabaseImpl&) = delete;
  CANDatabaseImpl(CANDatabaseImpl&&) = delete;
  CANDatabaseImpl& operator=(CANDatabaseImpl&&) = delete;

  CANDatabaseImpl(const std::string& filename)
    : filename_(filename), map_(), 
      intKeyIndex_(), strKeyIndex_() {

  }

  std::string filename_;
  container_type map_; // Index by CAN ID

  std::map<unsigned long long, IDKey> intKeyIndex_;
  std::map<std::string, IDKey> strKeyIndex_;
};

CANDatabase::CANDatabase()
  : impl(new CANDatabaseImpl()) {
}

CANDatabase::CANDatabase(const std::string& filename)
  : impl(new CANDatabaseImpl(filename)) { }

CANDatabase::CANDatabase(const CANDatabase& other)
  : impl(new CANDatabaseImpl(other.impl->filename_)) {

  impl->map_ = other.impl->map_;
  impl->intKeyIndex_ = other.impl->intKeyIndex_;
  impl->strKeyIndex_ = other.impl->strKeyIndex_;
}

CANDatabase& CANDatabase::operator=(const CANDatabase& other) {
  impl->filename_ = other.impl->filename_;
  impl->map_ = other.impl->map_;
  impl->intKeyIndex_ = other.impl->intKeyIndex_;
  impl->strKeyIndex_ = other.impl->strKeyIndex_;
  return *this;
}

CANDatabase::CANDatabase(CANDatabase&& other)
  : impl(nullptr) {
    swap(*this, other);
}

CANDatabase& CANDatabase::operator=(CANDatabase&& other) {
  swap(*this, other);
  return *this;
}

CANDatabase::~CANDatabase() {
  if(impl != nullptr)
    delete impl;
}

const std::string& CANDatabase::filename() const {
  return impl->filename_;
}

std::size_t CANDatabase::size() const {
  return impl->map_.size();
}

CANDatabase CANDatabase::fromFile(const std::string& filename, std::vector<parsing_warning>* warnings) {
  std::ifstream test_stream(filename);
  if (!test_stream.good()) {
    throw CANDatabaseException("Cannot find file " + filename);
  }
  
  dtl::FileTokenizer tokenizer(filename);
  return CppCAN::parser::dbc::fromTokenizer(filename, tokenizer, warnings);
}

CANDatabase CANDatabase::fromString(const std::string & src_string, std::vector<parsing_warning>* warnings) {
  dtl::StringTokenizer tokenizer(src_string);
  return CppCAN::parser::dbc::fromTokenizer(tokenizer, warnings);
}

const CANFrame& CANDatabase::at(const std::string& name) const {
  const IDKey& map_key = impl->strKeyIndex_.at(name);
  return impl->map_.at(map_key);
}

CANFrame& CANDatabase::at(const std::string& name) {
  const IDKey& map_key = impl->strKeyIndex_.at(name);
  return impl->map_.at(map_key);
}

const CANFrame& CANDatabase::at(unsigned long long id) const {
  const IDKey& map_key = impl->intKeyIndex_.at(id);
  return impl->map_.at(map_key);
}

CANFrame& CANDatabase::at(unsigned long long id) {
  const IDKey& map_key = impl->intKeyIndex_.at(id);
  return impl->map_.at(map_key);
}

void CANDatabase::addFrame(const CANFrame& frame) {
  IDKey map_key = { frame.name(), frame.can_id() };

  impl->map_.insert(std::make_pair(map_key, frame));
  impl->strKeyIndex_.insert(std::make_pair(frame.name(), map_key));
  impl->intKeyIndex_.insert(std::make_pair(frame.can_id(), map_key));
}

void CANDatabase::removeFrame(const std::string& name) {
  try {
    const IDKey& map_key = impl->strKeyIndex_.at(name);

    impl->map_.erase(impl->map_.find(map_key));
    impl->strKeyIndex_.erase(impl->strKeyIndex_.find(map_key.str_key));
    impl->intKeyIndex_.erase(impl->intKeyIndex_.find(map_key.int_key));
  } 
  catch(const std::out_of_range&) {
    std::string excepText = "Cannot remove frame with name " + name;
    throw std::out_of_range(excepText);
  }
}

void CANDatabase::removeFrame(unsigned int can_id) {
  try {
    const IDKey& map_key = impl->intKeyIndex_.at(can_id);

    impl->map_.erase(impl->map_.find(map_key));
    impl->strKeyIndex_.erase(impl->strKeyIndex_.find(map_key.str_key));
    impl->intKeyIndex_.erase(impl->intKeyIndex_.find(map_key.int_key));
  } 
  catch(const std::out_of_range&) {
    std::string excepText = "Cannot remove frame with CAN ID ";
    excepText += std::to_string(can_id);
    throw std::out_of_range(excepText);
  }
}

bool CANDatabase::contains(unsigned long long can_id) const {
  return impl->intKeyIndex_.find(can_id) != impl->intKeyIndex_.end();
}

bool CANDatabase::contains(const std::string& name) const {
  return impl->strKeyIndex_.find(name) != impl->strKeyIndex_.end();
}

CANDatabase::iterator 
CANDatabase::begin() {
  return impl->map_.begin();
}

CANDatabase::const_iterator 
CANDatabase::begin() const {
  return impl->map_.begin();
}

CANDatabase::const_iterator
CANDatabase::cbegin() const {
  return impl->map_.cbegin();
}

CANDatabase::iterator 
CANDatabase::end() {
  return impl->map_.end();
}

CANDatabase::const_iterator 
CANDatabase::end() const {
  return impl->map_.end();
}

CANDatabase::const_iterator
CANDatabase::cend() const {
  return impl->map_.cend();
}

CANDatabase::reverse_iterator 
CANDatabase::rbegin() {
  return impl->map_.rbegin();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rbegin() const {
  return impl->map_.rbegin();
}

CANDatabase::const_reverse_iterator
CANDatabase::crbegin() const {
  return impl->map_.crbegin();
}

CANDatabase::reverse_iterator 
CANDatabase::rend() {
  return impl->map_.rend();
}

CANDatabase::const_reverse_iterator 
CANDatabase::rend() const {
  return impl->map_.rend();
}

CANDatabase::const_reverse_iterator
CANDatabase::crend() const {
  return impl->map_.crend();
}

void CANDatabase::clear() {
  impl->map_.clear();
  impl->intKeyIndex_.clear();
  impl->strKeyIndex_.clear();
}

void CppCAN::swap(CANDatabase & first, CANDatabase & second) {
  std::swap(first.impl, second.impl);
}

const CANFrame& CANDatabase::operator[](unsigned long long can_id) const {
  const IDKey& map_key = impl->intKeyIndex_.at(can_id);
  return impl->map_.at(map_key);
}

CANFrame& CANDatabase::operator[](unsigned long long can_id) {
  const IDKey& map_key = impl->intKeyIndex_.at(can_id);
  return impl->map_.at(map_key);
}

const CANFrame& CANDatabase::operator[](const std::string& name) const {
  const IDKey& map_key = impl->strKeyIndex_.at(name);
  return impl->map_.at(map_key);
}

CANFrame& CANDatabase::operator[](const std::string& name) {
  const IDKey& map_key = impl->strKeyIndex_.at(name);
  return impl->map_.at(map_key);
}

bool CANDatabase::IntIDKeyCompare::operator()(const IDKey& k1, const IDKey& k2) const {
  return k1.int_key < k2.int_key;
}
