#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"

namespace soemdsp::modulator {

class LinearDADSR {
  public:
    enum class State {
        off,
        delay,
        attack,
        decay,
        sustain,
        release
    };

    // io
    void sampleRateChanged() {
        attackTimeChanged();
        sustainAmplitudeChanged();
    }
    double& getValue() {
        return out_;
    };
    State getState() {
        return state_;
    }
    double out_{};

    // parameters
    void attackTimeChanged() { // attackTime_
        attackIncrement_ = std::min(SampleRate::period_ / attackTime_, 1.0);
    }

    void decayTimeChanged() { // decayTime_
        decayDecrement_ = (1.0 - sustainAmp_) * SampleRate::period_ / decayTime_;
    }

    void sustainAmplitudeChanged() { // also updates decay and release time
        decayTimeChanged();
        releaseTimeChanged();
    }

    void releaseTimeChanged() { // releaseTime_
        releaseDecrement_ = out_ * SampleRate::period_ / releaseTime_;
    }
    Wire<double> sustainAmp_{ 1.0 };
    Wire<double> delayTime_;
    Wire<double> attackTime_;         // 1.e-6 to inf exponential
    Wire<double> decayTime_{ 1.0 };   // 1.e-6 to inf exponential
    Wire<double> releaseTime_{ 1.0 }; // 1.e-6 to inf exponential
    Wire<int> isLooping_;

    bool isIdle() {
        return state_ == State::off;
    }

    void triggerRelease() {
        state_ = State::release;
        releaseTimeChanged();
    }

    void reset() {
        out_           = 0.0;
        state_         = State::off;
        secondsPassed_ = 0.0;
    }

    void triggerAttack() {
        // skip delay stage if delay time is negligible
        if (delayTime_ < SampleRate::period_) {
            //  skip attack stage if attack time is negliglbe
            if (attackTime_ <= SampleRate::period_) {
                state_ = State::decay;
                out_   = 1.0;
            } else {
                state_ = State::attack;
            }
        } else {
            if (out_ <= 1.e-6) {
                reset();
            }
            state_ = State::delay;
        }
    }

    double run() {
        switch (state_) {
        case State::off:
            break;

        case State::delay: {
            secondsPassed_ += SampleRate::period_;
            if (secondsPassed_ >= delayTime_) {
                state_ = State::attack;
            }
        } break;

        case State::attack:
            out_ += attackIncrement_;

            if (out_ >= 1.0) {
                out_   = 1.0;
                state_ = State::decay;
            }
            break;

        case State::decay:
            out_ -= decayDecrement_;
            if (out_ <= sustainAmp_) {
                out_   = sustainAmp_;
                state_ = State::sustain;
            }
            break;

        case State::sustain:
            if (isLooping_) {
                state_ = State::attack;
            }
            out_ = sustainAmp_;
            break;

        case State::release: {
            out_ -= releaseDecrement_;
            if (out_ <= 0) {
                reset();
            }
        } break;
        }

        return out_;
    }

  private:
    /* Internal Values*/
    double peak_{};
    double attackIncrement_{};
    double decayDecrement_{};
    double releaseDecrement_{};

    /* Counters */
    double secondsPassed_{};

    bool skipDecayStage_{};
    State state_{};
};

class LinearDASR {
  public:
    enum class State {
        off,
        delay,
        attack,
        sustain,
        release
    };

#pragma region UPDATE
    // updates attack, sustain, and release
    void sampleRateChanged() {
        attackTimeChanged();
        releaseTimeChanged();
    }

    // 1.e-6 to inf exponential
    void attackTimeChanged() {
        attackIncrement_ = std::min(SampleRate::period_ / attackTime_, 1.0);
    }
    // 1.e-6 to inf exponential
    void releaseTimeChanged() {
        releaseDecrement_ = out_ * SampleRate::period_ / std::max<double>(releaseTime_, SampleRate::period_);
    }

    /* Parameters */
    Wire<double> delayTime_;
    Wire<double> attackTime_;
    Wire<int> sustainOffOn_;
    Wire<double> releaseTime_{ 1.0 };
    Wire<double> velocitySensitivity_{ 0.0 };
    Wire<double> velocity_{ 1.0 };

    /* Outputs */
    State getState() {
        return state_;
    }
#pragma endregion

    bool isIdle() {
        return state_ == State::off;
    }

    // this also calls releaseTimeChanged()
    void triggerRelease() {
        state_ = State::release;
        releaseTimeChanged();
    }

    void reset() {
        out_           = 0.0;
        state_         = State::off;
        secondsPassed_ = 0.0;
    }

    void triggerAttack() {
        peak_ = valFromVelocityAndSensitivity(velocity_, velocitySensitivity_);
        // skip delay stage if delay time is negligible
        if (delayTime_ < SampleRate::period_) {
            //  skip attack stage if attack time is negliglbe
            if (attackTime_ <= SampleRate::period_) {
                state_ = State::sustain;
                out_   = peak_;
            } else {
                state_ = State::attack;
            }
        } else {
            if (out_ <= 1.e-6) {
                reset();
            }
            state_ = State::delay;
        }
    }

    double run() {
        switch (state_) {
        case State::off:
            break;

        case State::delay: {
            secondsPassed_ += SampleRate::period_;
            if (secondsPassed_ >= delayTime_) {
                state_ = State::attack;
            }
        } break;

        case State::attack:
            out_ += attackIncrement_;

            if (out_ >= peak_) {
                out_   = peak_;
                state_ = State::sustain;
            }
            break;

        case State::sustain:
            out_ = peak_;
            if (sustainOffOn_ == 0 || isLooping_) {
                triggerRelease();
            }
            break;
        case State::release:
            out_ -= releaseDecrement_;
            if (out_ <= 0.0) {
                out_ = 0.0;
                if (isLooping_) {
                    state_ = State::attack;
                } else {
                    reset();
                }
            }
            break;
        }

        return out_;
    }

  private:
    /* Internal Values*/
    double attackIncrement_{};
    double releaseDecrement_{};
    double peak_{};

    /* Counters */
    double out_{};
    double secondsPassed_{};

    bool isLooping_{};
    bool skipDecayStage_{};
    State state_{};
};
} // namespace soemdsp::modulator
