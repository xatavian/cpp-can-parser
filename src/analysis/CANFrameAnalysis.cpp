#include "CANDatabaseAnalysis.h"
#include <algorithm>
#include <cmath>
#include <tuple>

// For each entry: (byte, lr start bit, lr end bit)
struct SignalRange {
    uint8_t byte;
    uint8_t lr_start_bit;
    uint8_t lr_end_bit;
};

using SignalRanges = std::vector<SignalRange>;

/**
 * Few explanations about the attributes:
 * lr_start_bit: Start bit position reading from left to right (ie bit 7 of byte 1 is 0, bit 7 of byte 2 is 9, ...)
 * lr_end_bit: same but for the end bit.
 */
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
    // of the signal. Therefore, it is only required to transform it into a "human-readable"
    // value, ie. when looking at the frame from left to right.
    // -----------------               -----------------
    // |*|*|*|*|*|*|*|*|               |*|*|*|*|*|*|*|*|
    // -----------------               -----------------
    //  0             7   instead of    7             0
    //
    // Example: Start bit 4 becomes LR start bit 3
    unsigned bitsAnalyzed = 0;
    bool is_start_byte = 0;
    
    for(unsigned current_byte = src.start_bit() / 8; bitsAnalyzed < src.length(); current_byte++, is_start_byte = false) {
        unsigned lbit = is_start_byte ? (7 - src.start_bit() % 8) : 0;
        unsigned rbit = std::min(7u, src.start_bit() - bitsAnalyzed);

        // The static_cast are not "necessary" but it removes some warnings
        result.push_back({ static_cast<uint8_t>(current_byte), 
                           static_cast<uint8_t>(lbit),
                           static_cast<uint8_t>(rbit) });
    }
    
    return result;
}

SignalRanges little_endian_ranges(const CANSignal& src) {
    SignalRanges result;

    // For LittleEndian signals, the start bit represents the LSB of the signal
    // which does not really represent anything in terms of layout. So we need
    // to compute the ranges for each byte individually anyway.

    unsigned bitsLeft = src.length();
    bool is_start_byte = 0;

    for(unsigned current_byte = src.start_bit() / 8; bitsLeft > 0; current_byte++, is_start_byte = false) {
        unsigned lbit = 7 -( src.start_bit() + bitsLeft);
        unsigned rbit = 7 - src.start_bit();

        // The static_cast are not "necessary" but it removes some warnings
        result.push_back({ static_cast<uint8_t>(current_byte), 
                           static_cast<uint8_t>(lbit),
                           static_cast<uint8_t>(rbit) });

        bitsLeft -= (rbit - lbit);
    }

    return result;
}

std::vector<SignalLayoutEntry> compute_layout(const CANFrame& src) {
    std::vector<SignalLayoutEntry> result;

    for(const auto& signal: src) {
        const CANSignal& sig = signal.second; 
        int lr_start_bit, lr_end_bit;

        if(sig.endianness() == CANSignal::BigEndian) {
            auto ranges = big_endian_ranges(sig);
            result.emplace_back(&sig, std::move(ranges));
        }
        else {
            auto ranges = little_endian_ranges(sig);
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
          return r.lr_start_bit < rr.lr_start_bit;
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
        const SignalLayoutEntry& e = layout[i];

        for(size_t j = i + 1; j < layout.size(); j++) {
            if(overlap(layout[i], layout[j]))
                return false;            
        }
    }

    return true;
}