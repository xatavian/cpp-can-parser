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
   * @brief A parsing warning and its location
   */
  struct parsing_warning {
    int line;
    std::string description;
  };

public:
  /**
   * @brief Parse a CANDatabase from the given source file.
   * @param filename Path to the file to parse
   * @param warnings (Optional) Filled with all the warnings found during the parsing
   * @throw CANDatabaseException if the parsing failed
   */
  static CANDatabase fromFile(
    const std::string& filename, std::vector<parsing_warning>* warnings = nullptr);

  /**
   * @brief Construct a CANDatabase object from a database described by src_string
   * @param src_string Source string to parse
   * @param warnings (Optional) Filled with all the warnings found during the parsing
   * @throw CANDatabaseException if the parsing failed
   */
  static CANDatabase fromString(
    const std::string& src_string, std::vector<parsing_warning>* warnings = nullptr);

public:
  struct IDKey {
    std::string str_key;
    unsigned long long int_key;
  };

  struct IntIDKeyCompare {
    bool operator()(const IDKey& k1, const IDKey& k2) const;
  };

  using container_type = std::map<IDKey, CANFrame, IntIDKeyCompare>;
  
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
  CANDatabase(const CANDatabase&) = default;

  /**
   * @brief Makes a copy of the given database
   */
  CANDatabase& operator=(const CANDatabase&) = default;

  /**
   * @brief Moves a CANDatabase object. The CANFrame objects are NOT deep copied.
   */
  CANDatabase(CANDatabase&&) = default;

  /**
   * @see CANDatabase(const CANDatabase&&)
   */
  CANDatabase& operator=(CANDatabase&&) = default;

public:
  /**
   * @brief Get the frame with the given frame name
   */
  const CANFrame& at(unsigned long long frame_id) const;
  
  /**
   * @brief Get the frame with the given frame name
   */
  CANFrame& at(unsigned long long frame_id);
  
  /**
   * @brief Get the frame with the given frame id
   */
  const CANFrame& at(const std::string& frame_name) const;
  
  /**
   * @brief Get the frame with the given frame id
   */
  CANFrame& at(const std::string& frame_name);

  /**
   * @brief Get the frame with the given frame id
   */
  const CANFrame& operator[](unsigned long long frame_idx) const;
  
  /**
   * @brief Get the frame with the given frame id
   */
   CANFrame& operator[](unsigned long long frame_idx);
  
  /**
   * @brief Get the frame with the given frame name
   */
  const CANFrame& operator[](const std::string& frame_name) const;
  
  /**
   * @brief Get the frame with the given frame name
   */
  CANFrame& operator[](const std::string& frame_name);

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

  void addFrame(const CANFrame& frame);
  void removeFrame(unsigned int idx);
  void removeFrame(const std::string& name);

private:
  std::string filename_;
  container_type map_; // Index by CAN ID

  std::map<unsigned long long, IDKey> intKeyIndex_;
  std::map<std::string, IDKey> strKeyIndex_;
};

#endif
