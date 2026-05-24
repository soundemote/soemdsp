#pragma once
#include <soemdsp/Wire.hpp>
#include "signalsmithdsp/delay.h"
#include "signalsmithdsp/envelopes.h"
#include "soemdsp/SampleRate.hpp"
namespace soemdsp::dynamics {
struct EnvelopeFollower {

    EnvelopeFollower() {
        sampleRateChanged();
        attackTimeChanged();
        holdTimeChanged();
        releaseTimeChanged();
    }

    struct ExponentialRelease {
        ExponentialRelease() {
            sampleRateChanged();
        }

        // io
        double run(double in) {
            // Move towards input
            out_ += (in - out_) * releaseSlew_;
            out_ = std::min(out_, in);
            return out_;
        }
        double out_{ 1.0 };

        // parameters
        void sampleRateChanged() { // releaseTime
            // The exact value is `1 - exp(-1/releaseSamples)`
            // but this is a decent approximation
            releaseSlew_ = 1.0 / (SampleRate::timeToSamples(releaseTime_) + 1.0);
        }
        Wire releaseTime_{ .004 };

        // coefficients
        double releaseSlew_{};
    };
    struct ConstantTimeRelease {
        ConstantTimeRelease() {
            sampleRateChanged();
        }

        // io
        void sampleRateChanged() { // releaseTime_
            releaseSamples_ = std::max(SampleRate::timeToSamples(releaseTime_), 1.0);
            // This will finish its release 0.01 samples too early
            // but that avoids numerical errors
            gradientFactor_ = 1.0 / (static_cast<double>(releaseSamples_) - 0.01);
            peakHold_.resize(releaseSamples_);
            peakHold_.reset(1); // start with gain 1
        }

        double run(double in) {
            // We need the peak from one sample back
            double prevMin = -peakHold_.read();
            peakHold_(-in);
            // Gradient is proportional to the difference
            out_ += (in - prevMin) * gradientFactor_;
            out_ = std::min(out_, in);
            return out_;
        }

        double out_{ 1 };

        // settings
        const double maxTime_{ 4.0 };

        // parameters
        double releaseTime_;

        // objects
        signalsmith::envelopes::PeakHold<double> peakHold_{ 0 };

      protected:
        // coefficients
        double gradientFactor_{ 1 };
        int releaseSamples_;
    };

    // io
    int latency() {
        return attackSamples_;
    }
    double run(double in) {
        double g = gain(in);
        // double delayedV = delay_.write(in).read(attackSamples_);
        out_ = g;
        return out_;
    }
    double out_;

    // settings
    const double MAX_TIME{ 4.0 }; // max time for attack, hold, and release
    const double MAX_SAMPLES{ (SampleRate::timeToSamples(MAX_TIME)) };

    // parameters
    void sampleRateChanged() {
        attackSamples_  = (SampleRate::timeToSamples(attackTime_));
        holdSamples_    = (SampleRate::timeToSamples(holdTime_));
        releaseSamples_ = (SampleRate::timeToSamples(releaseTime_));

        releaser_.releaseTime_  = releaseTime_;
        releaser2_.releaseTime_ = releaseTime_;
        releaser_.sampleRateChanged();
        releaser2_.sampleRateChanged();

        peakHold_.resize(MAX_SAMPLES * 2);
        smoother_.resize(MAX_SAMPLES, 3);
        smoother_.reset(1);

        delay_.resize((SampleRate::timeToSamples(MAX_TIME)) + 1);
    }
    void attackTimeChanged() {
        debug::WITHINRANGE<int>(attackTime_, 0, MAX_TIME);
        attackSamples_ = (SampleRate::timeToSamples(attackTime_));
        smoother_.set(attackSamples_);
        peakHold_.set(attackSamples_ + holdSamples_);
    }
    void holdTimeChanged() {
        debug::WITHINRANGE<int>(holdTime_, 0, MAX_TIME);
        holdSamples_ = (SampleRate::timeToSamples(holdTime_));
        peakHold_.set(attackSamples_ + holdSamples_);
    }
    void releaseTimeChanged() {
        debug::WITHINRANGE<int>(releaseTime_, 0, MAX_TIME);
        releaser_.releaseTime_  = releaseTime_;
        releaser2_.releaseTime_ = releaseTime_;
        releaser_.sampleRateChanged();
        releaser2_.sampleRateChanged();
    }
    Wire limit_{ 0.25 };      // 0.0 to 1.0, threshold in amplitude
    Wire attackTime_{ .005 }; // 0.0 to MAX_TIME
    Wire holdTime_{ .015 };   // 0.0 to MAX_TIME
    Wire releaseTime_{ .04 }; // 0.0 to MAX_TIME

    // objects
    signalsmith::envelopes::PeakHold<double> peakHold_{ 0 };
    signalsmith::envelopes::BoxStackFilter<double> smoother_{ 0 };
    signalsmith::delay::Delay<double> delay_; // We don't need fractional delays, so this could be nearest-sample
    ExponentialRelease releaser_;             // see the previous example code
    ExponentialRelease releaser2_;            // see the previous example code
  protected:
    // coefficients
    double attackSamples_{};
    double holdSamples_{};
    double releaseSamples_{};

    // double gain(double v) {
    //     double maxGain = 1;
    //     if (std::abs(v) > limit_) {
    //         maxGain = limit_ / std::abs(v);
    //     }
    //     double maxGainDb       = amp_to_db(maxGain);
    //     double movingMin       = -peakHold_(-maxGainDb);
    //     double releaseEnvelope = releaser_.run(movingMin);
    //     double outDb           = smoother_(releaseEnvelope);
    //     return db_to_amp(outDb);
    // }

    double gain(double in) {
        double maxGain = 1;
        double abs_v   = abs(in);
        if (abs_v > limit_) {
            maxGain = limit_ / abs_v;
        }
        double movingMin       = -peakHold_(-maxGain);
        double releaseEnvelope = releaser2_.run(releaser_.run(movingMin));
        return smoother_(releaseEnvelope);
    }
};
} // namespace soemdsp::dynamics
