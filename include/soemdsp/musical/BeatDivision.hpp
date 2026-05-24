#pragma once

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/utility/operations_string.hpp>
#include <soemdsp/musical/Transport.hpp>
#include <soemdsp/meta.hpp>

namespace soemdsp::musical {
struct BeatDivision {
    BeatDivision() = default;

    // io
    double out_{}; // final time including time offset

    // parameters: divison_, timeMultiplier_, modifier_
    // calls: timeOffsetChanged()
    void divisionChanged() {
        debug::WITHINSIZE(division_, noteDivisionStrings_);
        debug::WITHINSIZE(modifier_, noteModifiers_);

        beats_.w = noteDivisionValues_[static_cast<size_t>(division_)];
        beats_.w *= (modifier_ == 1 ? 1.5 : modifier_ == 2 ? constant::k2z3 : 1);
        beats_to_seconds_ = convert::beats_to_seconds(beats_);
        timeMultiplierChanged();
    }
    // parameters: timeMultiplier_
    // calls: timeOffsetChanged()
    void timeMultiplierChanged() {
        multipliedBeats_ = beats_to_seconds_ * timeMultiplier_;
        timeOffsetChanged();
    }
    // timeOffset_
    void timeOffsetChanged() {
        out_ = std::max(multipliedBeats_ + timeOffset_, SampleRate::period_);
    } 
    Wire division_;          // 0 = 1/128, 12 = 8/1
    Wire modifier_;          // 0 = straight, 1 = dotted, 2 = triplet
    Wire timeMultiplier_; // time multiplier to scale time with tempo
    Wire timeOffset_;     // amount of additional offset from calculated time

    // coefficients
    Wire beats_;
    Wire beats_to_seconds_;
    Wire multipliedBeats_;

    operator std::string() const {
        std::string mod;
        switch (convert::double_to_int(modifier_)) {
        case 0:
            break;
        case 1:
            mod = "D";
            break;
        case 2:
            mod = "T";
            break;
        default:
            debug::FAIL();
        }
        std::string mult   = str::limitDecimals(convert::double_to_string(timeMultiplier_), 4, 3, 3);
        std::string offset = str::limitDecimals(convert::double_to_string(timeOffset_), 4, 4, 4, true, true);
        return fmt::format("{} {} x{} + {}s", noteDivisionStrings_[static_cast<size_t>(division_)], mod, mult, offset);
    }

    std::vector<double> noteDivisionValues_       = { 1.0 / 128.0, 1.0 / 64.0, 1.0 / 32.0, 1.0 / 16.0, 1 / 8.0, 1.0 / 4.0, 1.0 / 2.0, 1.0, 2.0 / 1.0, 3.0 / 1.0, 4.0 / 1.0, 8.0 / 1.0 };
    std::vector<std::string> noteDivisionStrings_ = { "1/128", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "3/1", "4/1", "8/1" };
    std::vector<std::string> noteModifiers_       = { "", "D", "T" };
};

} // namespace soemdsp::musical
