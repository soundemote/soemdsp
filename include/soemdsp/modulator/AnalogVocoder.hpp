#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/semath.hpp>

namespace soemdsp::modulator {

// Analog-style vocoder core, made by VOID.
//
// Voice input controls the analysis envelopes.
// Carrier input provides the harmonic source that is reshaped by those envelopes.
// Character comes from band count, filter Q, and envelope timing, not digital
// damage such as bit crushing or sample-rate reduction.
class AnalogVocoder {
  public:
    static constexpr int maxBands_ = 16;

    class TptBandpass {
      public:
        void set(double sampleRate, double frequency, double q) {
            const double safeSampleRate = std::max(1.0, sampleRate);
            const double safeFrequency = std::clamp(frequency, 10.0, safeSampleRate * 0.45);
            const double safeQ = std::max(0.05, q);
            const double resonance = 1.0 / (2.0 * safeQ);

            g_ = std::tan(soemdsp::constant::kPI * safeFrequency / safeSampleRate);
            h_ = 1.0 / (1.0 + 2.0 * resonance * g_ + g_ * g_);
        }

        void reset() {
            ic1eq_ = 0.0;
            ic2eq_ = 0.0;
        }

        double run(double input) {
            const double v3 = input - ic2eq_;
            const double v1 = h_ * (ic1eq_ + g_ * v3);
            const double v2 = ic2eq_ + g_ * v1;

            ic1eq_ = 2.0 * v1 - ic1eq_;
            ic2eq_ = 2.0 * v2 - ic2eq_;

            return v1;
        }

      private:
        double g_{};
        double h_{ 1.0 };
        double ic1eq_{};
        double ic2eq_{};
    };

    class EnvelopeFollower {
      public:
        void set(double attackSeconds, double releaseSeconds) {
            attackCoefficient_  = coefficientFromSeconds(attackSeconds);
            releaseCoefficient_ = coefficientFromSeconds(releaseSeconds);
        }

        void reset() {
            value_ = 0.0;
        }

        double run(double input) {
            const double target = std::abs(input);
            const double coefficient = target > value_ ? attackCoefficient_ : releaseCoefficient_;
            value_ += (target - value_) * coefficient;
            return value_;
        }

      private:
        static double coefficientFromSeconds(double seconds) {
            if (!std::isfinite(seconds) || seconds <= 0.0) {
                return 1.0;
            }

            const double samples = std::max(1.0, soemdsp::SampleRate::timeToSamples(seconds));
            return 1.0 - std::exp(-1.0 / samples);
        }

        double attackCoefficient_{};
        double releaseCoefficient_{};
        double value_{};
    };

    AnalogVocoder() {
        sampleRateChanged();
    }

    void sampleRateChanged() {
        updateBands();
    }

    void parametersChanged() {
        updateBands();
    }

    void reset() {
        for (int i = 0; i < maxBands_; ++i) {
            analysis_[i].reset();
            synthesis_[i].reset();
            envelopes_[i].reset();
        }

        out_ = 0.0;
    }

    double run(double voiceIn, double carrierIn) {
        const int activeBands = activeBandCount();
        double wet = 0.0;

        for (int i = 0; i < activeBands; ++i) {
            const double voiceBand = analysis_[i].run(voiceIn);
            const double carrierBand = synthesis_[i].run(carrierIn);
            const double envelope = envelopes_[i].run(voiceBand);

            wet += carrierBand * envelope;
        }

        wet *= outputGain_ / static_cast<double>(std::max(1, activeBands));

        const double mix = std::clamp(static_cast<double>(dryWet_), 0.0, 1.0);
        out_ = voiceIn * (1.0 - mix) + wet * mix;
        return out_;
    }

    int activeBandCount() const {
        return std::clamp(static_cast<int>(std::round(static_cast<double>(bands_))), 1, maxBands_);
    }

    void updateBands() {
        const int activeBands = activeBandCount();
        const double low = std::max(10.0, static_cast<double>(lowFrequency_));
        const double high = std::max(low + 1.0, static_cast<double>(highFrequency_));
        const double q = std::max(0.05, static_cast<double>(filterQ_));

        for (int i = 0; i < maxBands_; ++i) {
            const int bandIndex = std::min(i, activeBands - 1);
            const double position = activeBands <= 1 ? 0.0 : static_cast<double>(bandIndex) / static_cast<double>(activeBands - 1);
            const double frequency = low * std::pow(high / low, position);

            analysis_[i].set(soemdsp::SampleRate::freq_, frequency, q);
            synthesis_[i].set(soemdsp::SampleRate::freq_, frequency, q);
            envelopes_[i].set(attackTime_, releaseTime_);
        }
    }

    double out_{};

    // Parameters.
    Wire bands_{ 8.0 };
    Wire lowFrequency_{ 120.0 };
    Wire highFrequency_{ 5000.0 };
    Wire filterQ_{ 1.0 };
    Wire attackTime_{ 0.006 };   // seconds
    Wire releaseTime_{ 0.180 };  // seconds
    Wire outputGain_{ 1.0 };
    Wire dryWet_{ 1.0 };

  private:
    std::array<TptBandpass, maxBands_> analysis_{};
    std::array<TptBandpass, maxBands_> synthesis_{};
    std::array<EnvelopeFollower, maxBands_> envelopes_{};
};

} // namespace soemdsp::modulator
