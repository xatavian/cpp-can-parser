#ifndef CANFrame_H
#define CANFrame_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "CANSignal.h"

class CANFrame {
public:
  CANFrame(const std::string& name, unsigned int can_id, unsigned int dlc);

  const std::string& name() const;
  unsigned int can_id() const;
  unsigned int dlc() const;
  unsigned int period() const;
  const std::string& comment() const;

  std::weak_ptr<CANSignal> getSignalByName(const std::string& name) const;
  std::weak_ptr<CANSignal> getSignalByStartBit(unsigned int start_bit) const;
  std::vector<std::weak_ptr<CANSignal>> signals() const;

  void addSignal(std::shared_ptr<CANSignal> signal);
  void removeSignal(unsigned int start_bit);
  void removeSignal(const std::string& name);

  bool hasSignal(const std::string& name) const;

  void setPeriod(unsigned int val);
  void setComment(const std::string& comment);
private:
  std::string name_;
  unsigned int can_id_;
  unsigned int dlc_;
  unsigned int period_;
  std::map<unsigned int, std::shared_ptr<CANSignal>> intIndex_; // Index by start bit
  std::map<std::string, std::shared_ptr<CANSignal>> strIndex_; // Index by name
  std::string comment_;
};

#endif
