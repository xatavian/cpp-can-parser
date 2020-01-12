#ifndef CANDATABASE_ANALYSIS_H
#define CANDATABASE_ANALYSIS_H

#include "CANFrame.h"

namespace CppCAN {
namespace analysis {
    /**
     * @brief Analyses the frame signals to see if some signals are overlapping
     * @return true if no overlapping is detected, false otherwise
     */
    bool is_frame_layout_ok(const CANFrame& src);

    /**
     * @brief Like is_frame_layout_ok() but throws a CANDatabaseException if the layout is ill-formed
     */
    void assert_frame_layout();
}    
} // namespace CppCAN

#endif