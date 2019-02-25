#include "CANFrame.h"
#include <utility>
#include <iostream>

CANFrame::CANFrame(const std::string& name, unsigned int can_id, unsigned int dlc)
  : name_(name), can_id_(can_id), dlc_(dlc), period_(0),
    intIndex_(), strIndex_() {}

const std::string& CANFrame::name() const {
  return name_;
}

unsigned int CANFrame::can_id() const {
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

bool CANFrame::hasSignal(const std::string& name) const {
  return strIndex_.find(name) != strIndex_.end();
}

std::weak_ptr<CANSignal> CANFrame::getSignalByName(const std::string& name) const {
  std::weak_ptr<CANSignal> result;

  auto ite = strIndex_.find(name);
  if(ite != strIndex_.end())
    result = ite->second;

  return result;
}

std::weak_ptr<CANSignal> CANFrame::getSignalByStartBit(unsigned int start_bit) const {
  std::weak_ptr<CANSignal> result;

  auto ite = intIndex_.find(start_bit);
  if(ite != intIndex_.end())
    result = ite->second;

  return result;
}

std::vector<std::weak_ptr<CANSignal>> CANFrame::signals() const {
  std::vector<std::weak_ptr<CANSignal>> result;

  for(const auto& sigIte: intIndex_) {
    result.emplace_back(sigIte.second);
  }
  return result;
}


void CANFrame::addSignal(std::shared_ptr<CANSignal> signal) {
  strIndex_.insert(std::make_pair(signal->name(), signal));
  intIndex_.insert(std::make_pair(signal->start_bit(), signal));
}

void CANFrame::removeSignal(const std::string& name) {
  auto ite = strIndex_.find(name);
  if(ite == strIndex_.end()) {
    std::cout << "WARNING: Cannot remove signal with name \""
              << name << "\" from frame \"" << this->name() << "\""
              << std::endl;
    return;
  }

  unsigned int intIdx = ite->second->start_bit();
  strIndex_.erase(ite); // No need for a second lookup
  intIndex_.erase(intIndex_.find(intIdx));
}

void CANFrame::removeSignal(unsigned int start_bit) {
  auto ite = intIndex_.find(start_bit);
  if(ite == intIndex_.end()) {
    std::cout << "WARNING: Cannot remove signal with start bit"
              << start_bit << " from frame \"" << this->name() << "\""
              << std::endl;
    return;
  }

  std::string strIdx = ite->second->name();
  intIndex_.erase(ite); // No need for a second lookup
  strIndex_.erase(strIndex_.find(strIdx));
}
