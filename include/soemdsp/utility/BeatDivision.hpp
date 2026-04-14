#pragma once

#include "Transport.hpp"
#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/operations_string.hpp"
#include "soemdsp/plugin/ParamHelpers.hpp"

namespace soemdsp::utility {
struct BeatDivision {
    SE_DEFAULT_CONSTRUCTOR(BeatDivision);

    // io
    double out_{}; // final time including time offset

    // parameters
    void divisionChanged();       // divison_, timeMultiplier_, modifier_, also calls timeOffsetChanged()
    void timeMultiplierChanged(); // timeMultiplier_
    void timeOffsetChanged();     // timeOffset_
    Wire<int> division_;           // 0 = 1/128, 12 = 8/1
    Wire<int> modifier_;           // 0 = straight, 1 = dotted, 2 = triplet
    Wire<double> timeMultiplier_;  // time multiplier to scale time with tempo
    Wire<double> timeOffset_;      // amount of additional offset from calculated time

    // coefficients
    Wire<double> beats_;
    Wire<double> beatsToTime_;
    Wire<double> multipliedBeats_;

    operator std::string() const {
        std::string mod;
        switch (modifier_) {
        case 0:
            break;
        case 1:
            mod = "D";
            break;
        case 2:
            mod = "T";
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        std::string mult   = str::limitDecimals(std::to_string(timeMultiplier_), 4, 3, 3);
        std::string offset = str::limitDecimals(std::to_string(timeOffset_), 4, 4, 4, true, true);
        return fmt::format("{} {} x{} + {}s", noteDivisionStrings_[toSizeT(division_)], mod, mult, offset);
    }

    std::vector<double> noteDivisionValues_       = { 1.0 / 128.0, 1.0 / 64.0, 1.0 / 32.0, 1.0 / 16.0, 1 / 8.0, 1.0 / 4.0, 1.0 / 2.0, 1.0, 2.0 / 1.0, 3.0 / 1.0, 4.0 / 1.0, 8.0 / 1.0 };
    std::vector<std::string> noteDivisionStrings_ = { "1/128", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "3/1", "4/1", "8/1" };
    std::vector<std::string> noteModifiers_       = { "", "D", "T" };
};
} // namespace soemdsp::utility
