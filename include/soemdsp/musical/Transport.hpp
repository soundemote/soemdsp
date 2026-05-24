#pragma once

#include <stdint.h>

#include <soemdsp/sehelper.hpp>

// update values using didChange functions which then returns true if the value did change
// this is a global class that dsp objects have access to.
namespace soemdsp::convert {
inline double beats_to_seconds(double beats, double tempo = 120.0) {
    return (60.0 / tempo) * beats;
}
inline double seconds_to_beats(double time, double tempo = 120.0) {
    return time * (tempo / 60.0);
}
inline double beats_to_freq(double beats, double tempo = 120.0) {
    return 1.0 / beats_to_seconds(beats, tempo);
}
inline double bars_to_freq(double bars, double tempo = 120.0) {
    return 1.0 / beats_to_seconds(bars * 4.0, tempo);
}
} // namespace soemdsp::convert
namespace soemdsp::musical {

struct Transport {
    Transport() = default;

    static inline bool isPlaying_{};
    static inline bool isRecording_{};

    static inline double tempo_{ 120.0 };
    static inline double tempoInc_{}; // tempo increment for each sample and until the next time info event

    static inline double positionBeats_{};
    static inline double positionSeconds_{};

    static inline double barStart_{};
    static inline int32_t barNumber_{};

    static inline double loopStartBeats_{};
    static inline double loopEndBeats_{};
    static inline double loopStartSeconds_{};
    static inline double loopEndSeconds_{};

    static inline double signatureNumerator_{ 4 };
    static inline double signatureDenominator_{ 4 };

    static bool didPlayingChange(bool v) {
        bool changed = isPlaying_ != v;
        isPlaying_   = v;
        return changed;
    }
    static bool didRecordingChange(bool v) {
        bool changed = isRecording_ != v;
        isRecording_ = v;
        return changed;
    }
    static bool didTempoChange(double v) {
        bool changed = tempo_ != v;
        tempo_       = v;
        return changed;
    }
    static bool didPositionChange(double positionInBeats, double positionInSeconds) {
        bool changed     = positionBeats_ != positionInBeats || positionSeconds_ != positionInSeconds;
        positionBeats_   = positionInBeats;
        positionSeconds_ = positionInSeconds;
        return changed;
    }
    static bool didBarChange(double barStart, int32_t barNumber) {
        bool changed = barStart_ != barStart || barNumber_ != barNumber;
        barStart_    = barStart;
        barNumber_   = barNumber;
        return changed;
    }
    static bool didLoopChange(double loopStartBeats, double loopEndBeats, double loopStartSeconds, double loopEndSeconds) {
        bool changed      = loopStartBeats_ != loopStartBeats || loopEndBeats_ != loopEndBeats;
        loopStartBeats_   = loopStartBeats;
        loopEndBeats_     = loopEndBeats;
        loopStartSeconds_ = loopStartSeconds;
        loopEndSeconds_   = loopEndSeconds;
        return changed;
    }
    static bool didSignatureChange(double numerator, double denominator) {
        bool changed          = signatureNumerator_ != numerator || signatureDenominator_ != denominator;
        signatureNumerator_   = numerator;
        signatureDenominator_ = denominator;
        return changed;
    }
};

} // namespace soemdsp::utility
