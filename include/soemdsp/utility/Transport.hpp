#pragma once

#include <stdint.h>
#include "soemdsp/sehelper.hpp"

namespace soemdsp::utility {
// update values using didChange functions which then returns true if the value did change
// this is a global class that dsp objects have access to.
struct Transport {
    SE_DEFAULT_CONSTRUCTOR(Transport);

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

    static double beatsToFrequency(double beats) {
        return 1.0 / beatsToTime(beats);
    }

    static double barsToFrequency(double bars) {
        return 1.0 / beatsToTime(bars * 4.0);
    }

    static double beatsToTime(double beats) {
        return (60.0 / tempo_) * beats;
    }

    static double timeToBeats(double time) {
        return time * (tempo_ / 60.0);
    }

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
        bool changed =
          loopStartBeats_ != loopStartBeats || loopEndBeats_ != loopEndBeats;
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

} // namespace soemdsp
