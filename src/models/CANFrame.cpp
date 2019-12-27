#include "CANFrame.h"
#include <utility>
#include <iostream>

CANFrame::CANFrame(const std::string& name, unsigned long long can_id, unsigned int dlc, unsigned int period, const std::string& comment)
  : name_(name), can_id_(can_id), dlc_(dlc), period_(0), comment_(comment),
    intIndex_(), strIndex_() {}

CANFrame::CANFrame(const CANFrame & src):
  name_(src.name_), can_id_(src.can_id_), dlc_(src.dlc_), 
  period_(src.period_), comment_(src.comment_),
  intIndex_(), strIndex_() {

  for (const auto& signal : src) {
    auto copy = std::make_shared<CANSignal>(*signal.second);
    intIndex_.insert(std::make_pair(signal.first, copy));
    strIndex_.insert(std::make_pair(signal.second->name(), copy));
  }
}

CANFrame& CANFrame::operator=(CANFrame src) {
  swap(*this, src);
  return *this;
}

CANFrame::CANFrame(CANFrame&& src) {
  swap(*this, src);
}

CANFrame& CANFrame::operator=(CANFrame&& src) {
  swap(*this, src);
  return *this;  
}

const std::string& CANFrame::name() const {
  return name_;
}

unsigned long long CANFrame::can_id() const {
  return can_id_;
}

unsigned int CANFrame::dlc() const {
  return dlc_;
}

unsigned int CANFrame::period() const {
  return period_;
}

void CANFrame::setPeriod(unsigned int val) {
  period_ = val;
}

void CANFrame::setComment(const std::string& comment) {
  comment_ = comment;
}

bool CANFrame::contains(const std::string& name) const {
  return strIndex_.find(name) != strIndex_.end();
}

std::shared_ptr<CANSignal> CANFrame::at(const std::string& name) const {
  return strIndex_.at(name);
}

std::shared_ptr<CANSignal> CANFrame::at(unsigned int start_bit) const {
  return intIndex_.at(start_bit);
}

void CANFrame::addSignal(std::shared_ptr<CANSignal> signal) {
  strIndex_.insert(std::make_pair(signal->name(), signal));
  intIndex_.insert(std::make_pair(signal->start_bit(), signal));
}

void CANFrame::removeSignal(const std::string& name) {
  auto ite = strIndex_.find(name);
  if(ite == strIndex_.end()) {
    std::string excepText = "Cannot remove signal with name \"" + name + 
                            "\" from frame \"" + this->name() + "\"";
    throw std::out_of_range(excepText);
  }

  unsigned int intIdx = ite->second->start_bit();
  strIndex_.erase(ite); // No need for a second lookup
  intIndex_.erase(intIndex_.find(intIdx));
}

CANFrame::iterator CANFrame::begin() {
  return intIndex_.begin();
}

CANFrame::const_iterator CANFrame::begin() const
{
  return intIndex_.begin();
}

CANFrame::const_iterator CANFrame::cbegin() const
{
  return intIndex_.cbegin();
}

CANFrame::iterator CANFrame::end()
{
  return intIndex_.end();
}

CANFrame::const_iterator CANFrame::end() const
{
  return intIndex_.end();
}

CANFrame::const_iterator CANFrame::cend() const
{
  return intIndex_.cend();
}

CANFrame::reverse_iterator CANFrame::rbegin()
{
  return intIndex_.rbegin();
}

CANFrame::const_reverse_iterator CANFrame::rbegin() const
{
  return intIndex_.rbegin();
}

CANFrame::const_reverse_iterator CANFrame::crbegin() const
{
  return intIndex_.crbegin();
}

CANFrame::reverse_iterator CANFrame::rend()
{
  return intIndex_.rend();
}

CANFrame::const_reverse_iterator CANFrame::rend() const
{
  return intIndex_.rend();
}

CANFrame::const_reverse_iterator CANFrame::crend() const
{
  return intIndex_.crend();;
}

std::size_t CANFrame::size() const
{
  return intIndex_.size();
}

void CANFrame::clear() {
  intIndex_.clear();
  strIndex_.clear();
}

void CANFrame::removeSignal(unsigned int start_bit) {
  auto ite = intIndex_.find(start_bit);
  if(ite == intIndex_.end()) {
    std::string excepText = "Cannot remove signal with start bit " + std::to_string(start_bit) +
                            "\" from frame \"" + this->name() + "\"";
    throw std::out_of_range(excepText);
  }

  std::string strIdx = ite->second->name();
  intIndex_.erase(ite); // No need for a second lookup
  strIndex_.erase(strIndex_.find(strIdx));
}

void swap(CANFrame & first, CANFrame & second) {
  std::swap(first.name_, second.name_);
  std::swap(first.can_id_, second.can_id_);
  std::swap(first.dlc_, second.dlc_);
  std::swap(first.period_, second.period_);
  std::swap(first.intIndex_, second.intIndex_);
  std::swap(first.comment_, second.comment_);
}
