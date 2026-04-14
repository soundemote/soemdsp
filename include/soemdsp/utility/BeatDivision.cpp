#include "BeatDivision.hpp"

using std::max;

namespace soemdsp::utility {
void BeatDivision::divisionChanged() {
    SE_WITHIN_SIZE(division_, noteDivisionStrings_);
    SE_WITHIN_SIZE(modifier_, noteModifiers_);

    beats_.w = noteDivisionValues_[toSizeT(division_)];
    beats_.w *= (modifier_ == 1 ? 1.5 : modifier_ == 2 ? k2z3 : 1);
    beatsToTime_ = Transport::beatsToTime(beats_);
    timeMultiplierChanged();
}
void BeatDivision::timeMultiplierChanged() {
    multipliedBeats_ = beatsToTime_ * timeMultiplier_;
    timeOffsetChanged();
}
void BeatDivision::timeOffsetChanged() {
    out_ = max(multipliedBeats_ + timeOffset_, SampleRate::period_);
}
} // namespace soemdsp::utility
