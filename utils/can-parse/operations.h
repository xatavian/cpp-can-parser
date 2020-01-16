#ifndef CPPCANPARSER_CAN_PARSE_OPERATIONS_H
#define CPPCANPARSER_CAN_PARSE_OPERATIONS_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>
#include "CANDatabase.h"

namespace CppCAN {
namespace can_parse {
    /**
     * @brief Prints a single frame selected by its CAN ID.
     */
    void print_single_frame(CANDatabase& db, uint32_t can_id);

    /**
     * @brief Prints all the frames of the database
     */
    void print_all_frames(CANDatabase& db);

    /**
     * @brief Checks the validity of all the frames of the database
     */
    void check_all_frames(CANDatabase& db, const std::vector<CANDatabase::parsing_warning>& warnings);

    /**
     * Exception thrown by any operation if an error happens during their
     * analysis.
     */
    struct CanParseException : std::runtime_error { 
        using std::runtime_error::runtime_error;
    };   
}
}

#endif // !CPPCANPARSER_CAN_PARSE_OPERATIONS_H
