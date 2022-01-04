#include "cpp-can-parser/CANDatabase.h"
#include <utility>
#include <iostream>

using namespace CppCAN;

CANFrame::CANFrame(const std::string& name, unsigned long long can_id, 
                   unsigned int dlc, unsigned int period, 
                   const std::string& comment)
  : name_(name), can_id_(can_id), dlc_(dlc), period_(0), comment_(comment) {}

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

const std::string& CANFrame::comment() const {
  return comment_;
}

void CANFrame::setPeriod(unsigned int val) {
  period_ = val;
}

void CANFrame::setComment(const std::string& comment) {
  comment_ = comment;
}

bool CANFrame::contains(const std::string& name) const {
  return map_.find(name) != map_.end();
}

const CANSignal& CANFrame::at(const std::string& name) const {
  return map_.at(name);
}

CANSignal& CANFrame::at(const std::string& name) {
  return map_.at(name); 
}

const CANSignal& CANFrame::operator[](const std::string& name) const {
  return at(name);
}

CANSignal& CANFrame::operator[](const std::string& name) {
  return at(name);
}

void CANFrame::addSignal(const CANSignal& signal) {  
  map_.insert(std::make_pair(signal.name(), signal));
}

void CANFrame::removeSignal(const std::string& name) {

  auto ite = map_.find(name);
  if(ite == map_.end()) {
    std::string excepText = "Cannot remove signal with name \"" + name + 
                            "\" from frame \"" + this->name() + "\"";
    throw std::out_of_range(excepText);
  }
  
  map_.erase(ite);
}

CANFrame::iterator CANFrame::begin() {
  return map_.begin();
}

CANFrame::const_iterator CANFrame::begin() const {
  return map_.begin();
}

CANFrame::const_iterator CANFrame::cbegin() const
{
  return map_.cbegin();
}

CANFrame::iterator CANFrame::end()
{
  return map_.end();
}

CANFrame::const_iterator CANFrame::end() const
{
  return map_.end();
}

CANFrame::const_iterator CANFrame::cend() const
{
  return map_.cend();
}

CANFrame::reverse_iterator CANFrame::rbegin()
{
  return map_.rbegin();
}

CANFrame::const_reverse_iterator CANFrame::rbegin() const
{
  return map_.rbegin();
}

CANFrame::const_reverse_iterator CANFrame::crbegin() const
{
  return map_.crbegin();
}

CANFrame::reverse_iterator CANFrame::rend()
{
  return map_.rend();
}

CANFrame::const_reverse_iterator CANFrame::rend() const
{
  return map_.rend();
}

CANFrame::const_reverse_iterator CANFrame::crend() const
{
  return map_.crend();;
}

std::size_t CANFrame::size() const
{
  return map_.size();
}

void CANFrame::clear() {
  map_.clear();
}

void CppCAN::swap(CANFrame & first, CANFrame & second) {
  std::swap(first.name_, second.name_);
  std::swap(first.can_id_, second.can_id_);
  std::swap(first.dlc_, second.dlc_);
  std::swap(first.period_, second.period_);
  std::swap(first.map_, second.map_);
  std::swap(first.comment_, second.comment_);
}