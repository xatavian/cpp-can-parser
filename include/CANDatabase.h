#ifndef CANDatabase_H
#define CANDatabase_H

#include <string>
#include <memory>
#include <exception>
#include <map>

#include "CANFrame.h"
#include "CANDatabaseException.h"

/**
 * @brief A CAN database object
 *
 * A CAN database is an object regrouping frames and their signals' descriptions.
 * 
 * Public API: 
 * - contains to check if a frame name/frame id is registered in the database
 * - at to get the frame associated with the given frame name/frame id
 *   the same operations are available with operator[]
 * - addFrame and removeFrame to alter the database's content
 *
 * If the database was parsed from a file, the filename() method can be used to
 * retrieve the name of the source file.
 */
class CANDatabase {
public:
  /**
   * @brief Parse a CANDatabase from the given source file.
   * @param filename Path to the file to parse
   * @throw CANDatabaseException if the parsing failed
   */
  static CANDatabase fromFile(const std::string& filename);

  /**
   * @brief Construct a CANDatabase object from a database described by src_string
   * @param src_string Source string to parse
   * @throw CANDatabaseException if the parsing failed
   */
  static CANDatabase fromString(const std::string& src_string);

public:
  using container_type = std::map<unsigned long long, std::shared_ptr<CANFrame>>;
  using str_container_type = std::map<std::string, std::shared_ptr<CANFrame>>;
  
  using iterator = container_type::iterator;
  using const_iterator = container_type::const_iterator;
  using reverse_iterator = container_type::reverse_iterator;
  using const_reverse_iterator = container_type::const_reverse_iterator;

public:
  /**
   * @brief Creates a CANDatabase object with no source file
   */
  CANDatabase() = default;

  /**
   * @brief Creates a CANDatabase object that has been constructed from a file
   * @param filename Name of the source file 
   */
  CANDatabase(const std::string& filename);

  /**
   * Creates a copy of the database: the individual frames are deep copied so there is no
   * shared memory betwwen the two databases.
   */
  CANDatabase(const CANDatabase&);

  /**
   * @brief Makes a copy of the given database
   * Note: the source database is passed-by-value for RVO
   *       (see https://stackoverflow.com/a/3279550/8147455 for more info)
   */
  CANDatabase& operator=(const CANDatabase&);

  /**
   * @brief Moves a CANDatabase object. The CANFrame objects are NOT deep copied.
   */
  CANDatabase(CANDatabase&&);

  /**
   * @see CANDatabase(const CANDatabase&&)
   */
  CANDatabase& operator=(CANDatabase&&);

public:
  /**
   * @brief Get the frame with the given frame name
   */
  std::shared_ptr<CANFrame> at(unsigned long long frame_name) const;
  
  /**
   * @brief Get the frame with the given frame id
   */
  std::shared_ptr<CANFrame> at(const std::string& frame_name) const;

  /**
   * @brief Get the frame with the given frame id
   * @see getFrame
   */
  std::shared_ptr<CANFrame> operator[](unsigned long long frame_idx) const;
  
  /**
   * @brief Get the frame with the given frame name
   */
  std::shared_ptr<CANFrame> operator[](const std::string& frame_name) const;

  /**
   * @return true if the CANDatabase contains a frame with the given frame id
   */
  bool contains(unsigned long long can_id) const;

  /**
   * @return true if the CANDatabase contains a frame with the given frame name
   */
  bool contains(const std::string& frame_name) const;

  /**
   * @brief Swaps the content of the two given databases
   */
  friend void swap(CANDatabase& first, CANDatabase& second);
  /**
   * @return File name of the source file if the database was constructed from a file.
   *         Otherwise, returns an empty string.
   */
  const std::string& filename() const;

  
  /* Set of methods used to behave like a STL container.
     Very useful for range-based for loops. Inspired from std::map but
     some features are missing. 
     
     The iterators have a random order. */
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

  void addFrame(std::shared_ptr<CANFrame> frame);
  void removeFrame(unsigned int idx);
  void removeFrame(const std::string& name);

private:
  std::string filename_;
  str_container_type strIndex_; // Index by frame name
  container_type intIndex_; // Index by CAN ID
};

#endif
