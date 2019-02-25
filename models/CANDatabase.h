#ifndef CANDatabase_H
#define CANDatabase_H

#include <string>
#include <memory>
#include <exception>
#include <map>

#include "CANFrame.h"
#include "CANDatabaseException.h"

class CANDatabase {
public:
  static CANDatabase fromFile(const std::string& filename);

public:
  CANDatabase(const std::string& filename);

public:
  std::weak_ptr<CANFrame> getFrameById(unsigned int idx) const;
  std::weak_ptr<CANFrame> getFrameByName(const std::string& name) const;
  std::vector<std::weak_ptr<CANFrame>> frames() const;

  const std::string& filename() const;
  std::size_t size() const;
  
  void addFrame(std::shared_ptr<CANFrame> frame);
  void removeFrame(unsigned int idx);
  void removeFrame(const std::string& name);

private:
  std::string filename_;
  std::map<std::string, std::shared_ptr<CANFrame>> strIndex_; // Index by frame name
  std::map<unsigned int, std::shared_ptr<CANFrame>> intIndex_; // Index by CAN ID
};

#endif
