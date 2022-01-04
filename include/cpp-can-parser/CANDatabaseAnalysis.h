#ifndef CANDATABASE_ANALYSIS_H
#define CANDATABASE_ANALYSIS_H

#include <vector>
#include "cpp-can-parser/CANDatabase.h"
#include "cpp_can_parser_export.h"

namespace CppCAN {
namespace analysis {
    /**
     * @brief Analyses the frame signals to see if some signals are overlapping
     * @param src The CANFrame instance to inspect
     * @return true if no overlapping is detected, false otherwise
     */
    CPP_CAN_PARSER_EXPORT bool is_frame_layout_ok(const CANFrame& src);

    /**
     * @brief Overload of is_frame_layout_ok() that outputs a diagnosis of the
     *        problematic signals if a layout error is detected.
     * @param src The CANFrame instance to inspect
     * @param diagnosis Filled with the names of all the overlapping signals
     * @return true if no overlapping is detected, false otherwise
     */
    CPP_CAN_PARSER_EXPORT bool is_frame_layout_ok(const CANFrame& src, std::vector<std::string>& diagnosis);

    /**
     * @brief Like is_frame_layout_ok() but throws a CANDatabaseException if the layout is ill-formed
     */
    CPP_CAN_PARSER_EXPORT void assert_frame_layout(const CANFrame& src);
}    
} // namespace CppCAN

#endif