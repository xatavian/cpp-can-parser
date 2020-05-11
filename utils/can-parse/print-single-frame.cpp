#include "operations.h"
#include "CANDatabase.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <sstream>

class ConsoleTable {
public:
  struct TableData {
    struct TableCell {
      enum Type {
        UnsignedHex, Unsigned, Signed, String, Float
      };

      Type type;
      uint64_t u;
      int64_t i;
      std::string s;
      double d;
    };

    std::vector<TableCell> data;
    bool is_full_line;
  };

  static const char LINE_SEPARATOR = '-';
  static const char COLUMN_SEPARATOR = '|';
  static const unsigned SPACE_AMOUNT = 2;

public:
  ConsoleTable(std::initializer_list<std::string> headers, bool add_final_line_separator = true) 
    : column_headers(headers), add_final_ls(add_final_line_separator) { 
      std::transform(column_headers.begin(), column_headers.end(), std::back_inserter(columns_width),
                     [](const std::string& s) { return s.size() + 2; });
    }

  ConsoleTable(unsigned headers_num, bool add_final_line_separator = true) {
    columns_width.resize(headers_num);
    add_final_ls = add_final_line_separator;
  }
  
  void add_row(std::initializer_list<TableData::TableCell> row_data, bool is_full_line) {
    data.push_back({ row_data, is_full_line });

    if(is_full_line)
      return;

    size_t i = 0;
    for(const TableData::TableCell& data: row_data) {
      if(data.type == TableData::TableCell::String) {
        columns_width[i] = std::max(data.s.size() + SPACE_AMOUNT, 
                                    columns_width[i]);
      }
      i++;
    }
  }

  void add_row(const std::vector<TableData::TableCell>& row_data, bool is_full_line) {
     data.push_back({ row_data, is_full_line });

     if(is_full_line)
      return;

    size_t i = 0;
    for(const TableData::TableCell& data: row_data) {
      if(data.type == TableData::TableCell::String) {
        columns_width[i] = std::max(data.s.size() + SPACE_AMOUNT, 
                                    columns_width[i]);
      }
      i++;
    }
  }

  void render() const {
    // Some columns headers have been defined, we can create the table normally
    if(column_headers.size() > 0) {
      unsigned total_width = std::accumulate(columns_width.begin(), columns_width.end(), 0);
      total_width += columns_width.size(); // for the + " "

      std::string line_separator(total_width, LINE_SEPARATOR);
          
      // Render the table header
      renderNormalHeader(line_separator);
    
      // Render the data
      for(size_t i = 0; i < data.size(); i++) {
        if(!data[i].is_full_line) {
          std::cout << COLUMN_SEPARATOR;
          for(size_t j = 0; j < data[i].data.size(); j++) {
            renderCell(data[i].data[j], columns_width[j]);
            std::cout << COLUMN_SEPARATOR;
          }
        }
        else {
          unsigned in_line = 0;
          bool prepare_new_line = true;
          bool prepare_end_line = false;
          for(size_t j = 0; j < data[i].data.size(); j++) {
            if(prepare_new_line) {
              std::cout << COLUMN_SEPARATOR << " ";
              in_line += 2;
              prepare_new_line = false;
            }

            if(in_line + data[i].data[j].s.size() < total_width - 2) {
              std::cout << data[i].data[j].s;
              in_line +=  data[i].data[j].s.size();
            }
            else {
              std::string substr = data[i].data[j].s.substr(0, total_width - in_line - 5);
              std::cout << substr << "...";

              in_line += substr.size() + 3;
              prepare_end_line = true;
            }

            if(j == data[i].data.size() - 1) {
              prepare_end_line = true;
            }
            else {
              in_line += 2;
              std::cout << ", ";
            }

            if(prepare_end_line) {
              std::cout << std::string(total_width - in_line, ' ') << COLUMN_SEPARATOR;
              prepare_end_line = false;
              prepare_new_line = true;
              in_line = 0;

              // Newline if there are still data to print
              if(j < data[i].data.size() - 1) {
                std::cout << std::endl;
              }
            }
          }
        }

        if(i < data.size() - 1 || add_final_ls) {
          std::cout << std::endl << line_separator << std::endl;
        }
      }
    }
    else {
      std::vector<std::string> computed_string;
      std::transform(data[0].data.begin(), data[0].data.end(), 
                     std::back_inserter(computed_string), [this](const TableData::TableCell& cell) {
          std::stringstream ss;
          ss << " ";
          parseCell(ss, cell);
          ss << " ";
          return ss.str();
      });

      size_t i = 0;
      unsigned total_width = std::accumulate(computed_string.begin(), computed_string.end(), 0, 
          [&i, this](unsigned prev, const std::string& s){ 
            return prev + s.size() + 1; 
          }
      );

      std::string line_separator(total_width, LINE_SEPARATOR);

      std::cout << line_separator << std::endl << COLUMN_SEPARATOR;
      
      for(size_t j = 0; j < data[0].data.size(); j++) {
        std::cout << computed_string[j] << COLUMN_SEPARATOR;
      }

      std::cout << std::endl;
      if(add_final_ls) {
        std::cout << line_separator << std::endl;
      }
    }
  }

private:
  void renderNormalHeader(const std::string& line_separator) const {
    std::cout << line_separator << std::endl << COLUMN_SEPARATOR;
    for(size_t i = 0; i < column_headers.size(); i++) {
      std::cout << std::left << std::setw(columns_width[i]) << (" " + column_headers[i]) << COLUMN_SEPARATOR; 
    }
    std::cout << std::endl << line_separator << std::endl;
  }

  void renderCell(const TableData::TableCell& cell, unsigned col_size) const {
        std::cout << std::left << " ";
        std::cout << std::setw(col_size - 1);

        parseCell(std::cout, cell);
  }

  void parseCell(std::ostream& is, const TableData::TableCell& cell) const {
    switch(cell.type) {
          case TableData::TableCell::Unsigned:
            is << cell.u;
            break;
          
          case TableData::TableCell::UnsignedHex:
            is << std::hex << std::showbase << cell.u
                      << std::dec << std::noshowbase;
            break;
          
          case TableData::TableCell::Signed:
            is << cell.i;
            break;

          case TableData::TableCell::String:
            is << cell.s;
            break;

          case TableData::TableCell::Float:
          {
            std::streamsize precision = is.precision();
            is << std::setprecision(3) << cell.d 
               << std::setprecision(precision);
          }
          break;
        }
  }
private:
  std::vector<std::string> column_headers;
  std::vector<size_t> columns_width;
  std::vector<TableData> data;
  bool add_final_ls;
};

ConsoleTable::TableData::TableCell createUnsigned(uint64_t val) {
  ConsoleTable::TableData::TableCell result;
  result.type = ConsoleTable::TableData::TableCell::Unsigned;
  result.u = val;

  return result;
}

ConsoleTable::TableData::TableCell createHex(uint64_t val) {
  ConsoleTable::TableData::TableCell result;
  result.type = ConsoleTable::TableData::TableCell::UnsignedHex;
  result.u = val;

  return result;
}

ConsoleTable::TableData::TableCell createStr(const std::string& val) {
  ConsoleTable::TableData::TableCell result;
  result.type = ConsoleTable::TableData::TableCell::String;
  result.s = val;

  return result;
}

ConsoleTable::TableData::TableCell createFloat(double val) {
  ConsoleTable::TableData::TableCell result;
  result.type = ConsoleTable::TableData::TableCell::Float;
  result.d = val;

  return result;
}

void print_frame_impl(const CANFrame& frame) {   
  std::cout << frame.name() << ":\t"  
            << std::hex << std::showbase << frame.can_id() << "/" 
            << std::dec << std::noshowbase << frame.dlc() << "/"
            << frame.period() << "ms" << std::endl;

  if(frame.comment().size() > 0) {
    std::cout << "COMMENT" << frame.name() << ":\t \"" << frame.comment() << "\"" << std::endl;
  }
}

std::vector<ConsoleTable::TableData::TableCell> createSignalChoicesVector(const CANSignal& sig) {
  std::vector<ConsoleTable::TableData::TableCell> result;
  
  for(const auto& choice: sig.choices()) {
    result.push_back(
      createStr(std::to_string(choice.first) + " -> \"" + choice.second + "\"")
    );
  }

  return result;
}

void CppCAN::can_parse::print_single_frame(CANDatabase& db, uint32_t can_id) {
  const CANFrame& frame = db[can_id];

  std::stringstream can_id_ss;
  can_id_ss << "CAN ID: " << std::hex << std::showbase << frame.can_id();

  ConsoleTable summary_header(4, false);
  summary_header.add_row({
    createStr(frame.name()), createStr(can_id_ss.str()), 
    createStr("DLC: " + std::to_string(frame.dlc())), createStr("Period: " + std::to_string(frame.period()) + "ms")
  }, false);

  ConsoleTable console_table = {
    "Signal name", "Start bit", "Length", "Scale", 
    "Offset", "Signedness", "Endianness", "Range"
  };
 
  for(const auto& sig : frame) {
    const CANSignal& signal = sig.second;
    console_table.add_row({
      createStr(signal.name()), 
      createUnsigned(signal.start_bit()), 
      createUnsigned(signal.length()),
      createFloat(signal.scale()), 
      createFloat(signal.offset()),
      signal.signedness() == CANSignal::Signed ? createStr("Signed") : createStr("Unsigned"),
      signal.endianness() == CANSignal::BigEndian ? createStr("BigEndian") : createStr("LittleEndian"),
      createStr("[" + std::to_string(signal.range().min) + ", " + std::to_string(signal.range().max) + "]")
    }, false);            

    if(signal.choices().size() > 0) {
      console_table.add_row(createSignalChoicesVector(signal), true);
    }
  }

  summary_header.render();
  console_table.render();
}