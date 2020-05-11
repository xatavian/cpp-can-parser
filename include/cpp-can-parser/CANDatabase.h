#ifndef CANDatabase_H
#define CANDatabase_H

#include <string>
#include <memory>
#include <stdexcept>
#include <map>
#include <vector>

#include "cpp_can_parser_export.h"

namespace CppCAN {

/**
 * @brief A CAN signal of a frame in the CAN Database
 * A CANSignal is represented by the following attributes:
 * - Name
 * - Start bit
 * - Length
 * - Scale
 * - Offset
 * - Endianness
 * - Signedness
 * - Minimum (optional)
 * - Maximum (optional)
 * - Comment (optional)
 * - Choices (optional) : map of unsigned int -> std::string (so one can associate 
 *                        a string value to an integer value)
 * 
 * All the attributes except for the comment and choices must be defined at the instanciation
 * and are immutable.
 */
class CPP_CAN_PARSER_EXPORT CANSignal {
public:
  struct CPP_CAN_PARSER_EXPORT Range {
    static Range fromString(const std::string& minstr, const std::string& maxstr);

    Range() = default; 
    Range(long m, long mm);

    bool defined;
    long min;
    long max;
  };

  enum Signedness {
    Unsigned, Signed
  };

  enum Endianness {
    BigEndian, LittleEndian
  };

public:
  CANSignal() = delete;
  CANSignal(const std::string& name, unsigned int start_bit, unsigned int length,
            double scale, double offset, Signedness signedness, Endianness endianness, Range range = Range());
  CANSignal(const CANSignal&) = default;
  CANSignal(CANSignal&&) = default;
  CANSignal& operator=(const CANSignal&) = default;
  CANSignal& operator=(CANSignal&&) = default;

  const std::string& name() const;

  unsigned int start_bit() const;

  unsigned int length() const;

  const std::string& comment() const;

  double scale() const;

  double offset() const;

  const Range& range() const;

  Signedness signedness() const;

  Endianness endianness() const;

  const std::map<unsigned int, std::string>& choices() const;

  void setComment(const std::string& comment);

  void setChoices(const std::map<unsigned int, std::string>& choices);

private:
  std::string name_;
  unsigned int start_bit_;
  unsigned int length_;
  double scale_;
  double offset_;
  Signedness signedness_;
  Endianness endianness_;
  Range range_;
  std::string comment_;
  std::map<unsigned int, std::string> choices_;
};

/**
 * @brief Object that gathers all the properties of a single frame. 
 * 
 * A CANFrame instance is characterized by the following properties:
 * - Name
 * - CAN ID
 * - DLC
 * - Period (optional)
 * - Comment (optional)
 * - List of signals
 * 
 * The name, CAN ID and DLC must be defined at the instanciation and are immutable.
 * The comment and period can respectivelly be changed with setComment() and setPeriod().
 * The list of signals can be modified with addSignal() and removeSignal(). Use clear()
 * to empty the signals' list.
 * 
 * One can access the CANSignal with at() or operator[]. **Be careful as both will throw
 * a std::out_of_range if the given key does noy match any signal in the frame.** To check
 * if a signal is present, use contains(). Signals can be found both by their start bit 
 * or their name.
 * 
 * CANFrame also behaves like a regular iterable: it defines the functions begin(), end(), 
 * cbegin(), cend(), ... You can traverse all the signals of the frame in a range-for loop
 * or use the standard library's algorithms !
 */
class CPP_CAN_PARSER_EXPORT CANFrame {
public:
  using container_type = std::map<std::string, CANSignal>;
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
};

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
class CPP_CAN_PARSER_EXPORT CANDatabase {
public:
  /**
   * @brief A parsing warning and its location
   */
  struct CPP_CAN_PARSER_EXPORT parsing_warning {
    unsigned long long line;
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
  struct CPP_CAN_PARSER_EXPORT IDKey {
    std::string str_key;
    unsigned long long int_key;
  };

  struct CPP_CAN_PARSER_EXPORT IntIDKeyCompare {
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
  CANDatabase();

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
   */
  CANDatabase& operator=(const CANDatabase&);

  /**
   * @brief Moves a CANDatabase object. The CANFrame objects are NOT deep copied.
   */
  CANDatabase(CANDatabase&&);

  /**
   * @see CANDatabase(CANDatabase&&)
   */
  CANDatabase& operator=(CANDatabase&&);

  ~CANDatabase();

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
  class CANDatabaseImpl;
  CANDatabaseImpl* impl;
};

/**
 * @brief Exception type for the library's operations
 */
class CANDatabaseException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}
#endif
