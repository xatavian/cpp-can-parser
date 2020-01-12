#ifndef CANFrame_H
#define CANFrame_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "CANSignal.h"

/**
 * @brief Object that gathers all the properties of a single frame. 
 */
class CANFrame {
public:
  struct IDKey {
    std::string str_key;
    unsigned long long int_key;
  };

  struct IntIDKeyCompare {
    bool operator()(const IDKey& k1, const IDKey& k2) const;
  };


  using container_type = std::map<IDKey, CANSignal, IntIDKeyCompare>;
  using iterator = container_type::iterator;
  using const_iterator = container_type::const_iterator;
  using reverse_iterator = container_type::reverse_iterator;
  using const_reverse_iterator = container_type::const_reverse_iterator;

public:
  // You cannot construct an empty frame.
  CANFrame() = delete;

  /**
   * @brief Construct a new frame.
   * @param name Name of the frame
   * @param can_id CAN ID of the frame
   * @param dlc DLC of the frame
   * @param comment Optional comment for the frame
   */
  CANFrame(const std::string& name, unsigned long long can_id, unsigned int dlc, 
           unsigned int period = 0, const std::string& comment = "");

  CANFrame(const CANFrame&) = default;
  CANFrame& operator=(const CANFrame&) = default;
  CANFrame(CANFrame&&) = default;
  CANFrame& operator=(CANFrame&&) = default;

public:
  /**
   * @return The name of the frame
   */
  const std::string& name() const;

  /**
   * @return The CAN ID of the frame
   */
  unsigned long long can_id() const;

  /**
   * @return The DLC of the frame
   */
  unsigned int dlc() const;

  /**
   * @return The period of the frame (If unspecified, then return 0)
   */
  unsigned int period() const;

  /**
   * @return The comment associated with the frame (if unspecified, return an empty string)
   */
  const std::string& comment() const;

public:
  /**
   * @brief Sets a new value for the frame's period.
   */
  void setPeriod(unsigned int val);

  /**
   * @brief Updates the frame's associated comment.
   */
  void setComment(const std::string& comment);

public:
  /**
   * @brief Fetches the signal with the given name.
   * @see at
   */
  const CANSignal& operator[](const std::string& name) const;
  
  /**
   * @brief Fetches the signal with the given name.
   * @see at
   */
  CANSignal& operator[](const std::string& name);

  /**
   * @brief Fetches the signal with the given name.
   */
  const CANSignal& at(const std::string& name) const;
  
  /**
   * @brief Fetches the signal with the given name.
   */
  CANSignal& at(const std::string& name);
  
  /**
   * @return true if a signal with the given name is already registered with the current frame.
   */
  bool contains(const std::string& name) const;

  /**
   * @brief Registers the given signal with the frame.
   */
  void addSignal(const CANSignal& signal);

  /**
   * @brief Removes the signal with the given start bit 
   */
  void removeSignal(unsigned int start_bit);

  /**
   * @brief Removes the signal associated with the given name
   */
  void removeSignal(const std::string& name);

public:
  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;

  iterator end();
  const_iterator end() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;

  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;

  std::size_t size() const;

  void clear();

  friend void swap(CANFrame& first, CANFrame& second);

private:
  std::string name_;
  unsigned long long can_id_;
  unsigned int dlc_;
  unsigned int period_;
  std::string comment_;
  
  container_type map_;
  std::map<unsigned, IDKey> intKeyIdx_; // Index by start bit
  std::map<std::string, IDKey> strKeyIdx_; // Index by name
};

#endif
