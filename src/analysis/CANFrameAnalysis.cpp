#include "cpp-can-parser/CANDatabaseAnalysis.h"
#include "cpp-can-parser/CANDatabase.h"
#include <algorithm>
#include <cmath>
#include <tuple>
#include <set>

using namespace CppCAN;

/**
 * For each entry: byte, [lr_start_bit, lr_end_bit)
 * Few explanations about the attributes:
 * lr_start_bit: Start bit position in the byte (included)
 * lr_end_bit: End bit position in the byte (excluded)
 */
struct SignalRange {
    uint8_t byte;
    char lr_start_bit;
    char lr_end_bit;
};

using SignalRanges = std::vector<SignalRange>;

static void
convert_to_big_endian_layout(SignalRanges& src) {
    // src is assumed to be LittleEndian
    for(SignalRange& r : src) {
        r.lr_start_bit = 7 - r.lr_start_bit;
        r.lr_end_bit = 7 - r.lr_end_bit;
    }
}

struct SignalLayoutEntry {
    SignalLayoutEntry() = delete;
    SignalLayoutEntry(const CANSignal* src, SignalRanges&& r)
        : src_signal(src), ranges(r) {

    }

    SignalLayoutEntry(const SignalLayoutEntry&) = default;
    SignalLayoutEntry& operator=(const SignalLayoutEntry&) = default;
    SignalLayoutEntry(SignalLayoutEntry&&) = default;
    SignalLayoutEntry& operator=(SignalLayoutEntry&&) = default;

    const CANSignal* src_signal;
    SignalRanges ranges;
};

SignalRanges big_endian_ranges(const CANSignal& src) {
    SignalRanges result;
    
    // For BigEndian signals, the start bit already represents the left mostbit 
    // -----------------  -----------------
    // |*|*|*|*|*|*|*|*|  |*|*|*|*|*|*|*|*|
    // -----------------  -----------------
    //  7             0    15            8
    
    unsigned bitsLeft = src.length();
    unsigned currentPos = src.start_bit();
    
    for(unsigned current_byte = src.start_bit() / 8; bitsLeft > 0; current_byte++) {
        char lbit = currentPos % 8;
        char rbit = std::max<char>(-1, lbit - bitsLeft);

        // The static_cast are not "necessary" but it removes some warnings
        result.push_back({ static_cast<uint8_t>(current_byte), 
                           lbit, rbit });
        
        bitsLeft -= lbit - rbit;
        currentPos += (lbit - rbit); 
    }
    
    return result;
}

SignalRanges little_endian_ranges(const CANSignal& src) {
    // For LittleEndian signals, act like the bits are reversed in the byte:
    // ----------------- -----------------
    // |*|*|*|*|*|*|*|*| |*|*|*|*|*|*|*|*|
    // ----------------- -----------------
    //  0             7   8             15    
    // 
    // The signal can be found from the start bit + read to the right.
    SignalRanges result;

    if(src.length() == 0) // length is 0, we return an empty result.
        return result;
    
    unsigned bitsLeft = src.length();
    unsigned currentPos = src.start_bit();
    for(unsigned current_byte = src.start_bit() / 8; bitsLeft > 0; current_byte++) {
        char lbit = currentPos % 8;
        char rbit = std::min<char>(lbit + bitsLeft, 8);

        // The static_cast are not "necessary" but it removes some warnings
        result.push_back({ static_cast<uint8_t>(current_byte), 
                           lbit, rbit });

        bitsLeft -= rbit - lbit;
        currentPos += rbit - lbit;
    }

    return result;
}

std::vector<SignalLayoutEntry> compute_layout(const CANFrame& src) {
    std::vector<SignalLayoutEntry> result;

    for(const auto& signal: src) {
        const CANSignal& sig = signal.second; 

        if(sig.endianness() == CANSignal::BigEndian) {
            auto ranges = big_endian_ranges(sig);
            result.emplace_back(&sig, std::move(ranges));
        }
        else {
            auto ranges = little_endian_ranges(sig);
            convert_to_big_endian_layout(ranges);
            result.emplace_back(&sig, std::move(ranges));
        }
    }

    return result;
}

bool overlap(const SignalLayoutEntry& e1, const SignalLayoutEntry& e2) {
  for(const SignalRange& r1 : e1.ranges) {
    for(const SignalRange& r2: e2.ranges) {
      // Find if r2 shares a SignalRange with the same byte with r1 
      if(r1.byte != r2.byte)
          continue;
        
      // Now we know that the SignalRange(s) share a common byte
        
      // ordered.first is the leftmost SignalRange in the byte
      // ordered.second is the rightmost SignalRange in the byte
      auto ordered = std::minmax(r1, r2, [](const SignalRange& r, const SignalRange& rr) {
          return r.lr_start_bit > rr.lr_start_bit;
      });

      // No overlapping if the last bit of the leftmost is before the first
      // bit of the rightmost.
      if(ordered.first.lr_end_bit < ordered.second.lr_start_bit)
        return true;
    }
  }

  return false;
}

bool CppCAN::analysis::is_frame_layout_ok(const CANFrame& src) {
    auto layout = compute_layout(src);

    for(size_t i = 0; i < layout.size(); i++) {
        for(size_t j = i + 1; j < layout.size(); j++) {
            if(overlap(layout[i], layout[j])) {
                return false;            
            }
        }
    }

    return true;
}

bool CppCAN::analysis::is_frame_layout_ok(const CANFrame& src, std::vector<std::string>& diagnosis) {
    auto layout = compute_layout(src);
    diagnosis.clear();

    std::set<size_t> diagnosis_indices;
    auto report_issue = [&diagnosis, &diagnosis_indices](size_t idx, const CANSignal& sig) {
        if(diagnosis_indices.count(idx) == 0) {
            diagnosis_indices.insert(idx);
            diagnosis.push_back(sig.name());
        }
    };

    for(size_t i = 0; i < layout.size(); i++) {
        for(size_t j = i + 1; j < layout.size(); j++) {
            if(overlap(layout[i], layout[j])) {
                report_issue(i, *layout[i].src_signal);
                report_issue(j, *layout[j].src_signal);
            }        
        }
    }

    return diagnosis_indices.size() == 0;
}

void CppCAN::analysis::assert_frame_layout(const CANFrame& src) {
    if(!is_frame_layout_ok(src)) {
        std::string text = "assert_frame_layout() failed for frame \"" + src.name() + "\"";
        throw CANDatabaseException(text);
    }
}