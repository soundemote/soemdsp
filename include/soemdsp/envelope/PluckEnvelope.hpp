#pragma once

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/utility/curve_functions.hpp>

namespace soemdsp::modulator {

class PluckEnvelope {
  public:
    enum class State {
        off,
        delay,
        attack,
        decay,
        release
    };

#pragma region UPDATE
    void sampleRateChanged() {
        updateAttackIncrement();
        updateDecayIncrement();
        updateReleaseIncrement();
    }

    void releaseFeedbackChanged() {
        fbReleaseAmp_ = std::min(kMaxFeedback_, exp(-fbRelease_ * 10.0));
    }

    void decayModFrequencyChanged() {
        phasorIncrement_ = decayModFrequency_ / SampleRate::freq_;
    }

    void autoReleaseTimeChanged() {
        if (autoReleaseTime_ <= kMinValue) {
            doAutoRelease = false;
        } else {
            doAutoRelease         = true;
            autoReleaseIncrement_ = SampleRate::timeToIncrement(std::max<double>(autoReleaseTime_, kMinValue));
        }
    }

    /* Parameters */
    Wire delayTime_;                  // Delay: delay time in seconds
    Wire attackFeedback_;             // Attack: attack time in feedback value
    Wire decay_;                      // Decay: 0.1 to 1.0
    Wire decayModStart_;              // AttackEnergy: .001 to 1.8
    Wire decayModEnd_;                // DecayEnergy: .010 to 3.0
    Wire endingDecay_;                // Sustain: 0.0 to 1.4
    Wire decayModCurve_;              // EnvelopeCurve: -1 to 1
    Wire decayModFrequency_;          // EnvelopeFrequency: 0.0 to 100.0 [update with decayModFrequencyChanged]
    Wire fbRelease_;                  // Release: 0.0 to 1.0 [update with releaseFeedbackChanged]
    Wire velocitySensitivity_{ 0.0 }; // 0.0 to 1.0
    Wire velocity_{ 1.0 };            // 0.0 to 1.0
    Wire autoReleaseTime_;            // AutoReleaseTime: in seconds 0.0 to 0.2 [update with autoReleaseTimeChanged]

    /* Coefficients */
    Wire fbAttackAmp_;
    Wire fbDecayAmp_;
    Wire fbReleaseAmp_;
    Wire phasorIncrement_;
#pragma endregion

    bool isIdle() {
        return state_ == State::off;
    }

    void triggerAttack() {
        peak_ = math::valFromVelocityAndSensitivity(velocity_, velocitySensitivity_);

        state_ = State::delay;

        // skip delay stage if delay time is negligible
        if (delayTime_ < SampleRate::period_) {
            // skip attack stage if attack is instantaneous
            if (attackFeedback_ <= kMinValue) {
                state_ = State::decay;
                prepareForDecay();
            } else {
                state_ = State::attack;
                updateAttackIncrement();
            }
        }
    }

    void reset() {
        currentValue_  = 0.0;
        state_         = State::off;
        secondsPassed_ = 0.0;
        phasor_        = 0.0;
    }

    void triggerRelease() {
        if (state_ != State::release) {
            state_ = State::release;
            updateReleaseIncrement();
        }
    }

    double run() {
        switch (state_) {
        case State::off:
            break;
        case State::delay:
            secondsPassed_ += SampleRate::period_;
            if (secondsPassed_ >= delayTime_) {
                state_ = State::attack;
                updateAttackIncrement();
            }
            break;
        case State::attack: {
            updateAttackIncrement();
            currentValue_ += SampleRate::period_ + currentValue_ * fbAttackAmp_;
            if (currentValue_ >= peak_) {
                state_ = State::decay;
                prepareForDecay();
            }
        } break;
        case State::decay:
            fbDecayAmp_.w = calculateFbDecayAmp();
            currentValue_ -= decayIncrement_ + currentValue_ * currentValue_ * fbDecayAmp_;
            phasor_ += phasorIncrement_;
            autoReleasePhasor_ += autoReleaseIncrement_;

            if (doAutoRelease && autoReleasePhasor_ >= 1.0) {
                triggerRelease();
            }

            if (currentValue_ < 0.0) {
                reset();
            }
            break;
        case State::release:
            currentValue_ -= releaseIncrement_ + currentValue_ * currentValue_ * fbReleaseAmp_;
            if (currentValue_ <= 0.0) {
                reset();
            }
            break;
        }

        return currentValue_;
    }

  private:
    double calculateFbDecayAmp() {
        if (phasor_ < 1.0) {
            double dmc = std::clamp<double>(decayModCurve_, -.99, .99);
            if (dmc == 0.0) {
                dmc = -1.e-8;
            };
            finalDecayMod_ = decay_ + math::map0to1<double>(curve::Exponential{ dmc }.get(phasor_), decayModStart_, decayModEnd_);
        } else {
            finalDecayMod_ = endingDecay_;
        }
        return std::min(kMaxFeedback_, exp(-finalDecayMod_ * 10.0));
    }

    void updateAttackIncrement() {
        fbAttackAmp_ = SampleRate::timeToIncrement(std::max<double>(attackFeedback_, kMinValue));
    }
    void updateDecayIncrement() {
        decayIncrement_ = (currentValue_ - 1.0) * SampleRate::period_ / 50.0;
    }
    void updateReleaseIncrement() {
        releaseIncrement_ = currentValue_ * SampleRate::period_ / 50.0;
    }

    void prepareForDecay() {
        phasor_            = 0.0;
        autoReleasePhasor_ = 0.0;
        currentValue_      = peak_;
        updateDecayIncrement();
    }

    State state_{};

    double decayIncrement_{};
    double finalDecayMod_{};
    double releaseIncrement_{};
    double autoReleaseIncrement_{};

    double currentValue_{};
    double secondsPassed_{};

    double peak_{};
    double phasor_{};
    double autoReleasePhasor_{};

    bool doAutoRelease{};

    const double kMinValue     = 1.e-8;
    const double kMaxFeedback_ = 1.0 - 1.e-6; // std::nextafter(1.0, 0); // 0.999999;
};

// class PluckEnvelopeWithLogAttack {
//   public:
//     enum class State {
//         off,
//         delay,
//         attack,
//         decay,
//         release
//     };
//
// #pragma region UPDATE
//     void sampleRateChanged() {
//         updateAttackIncrement();
//         updateDecayIncrement();
//         updateReleaseIncrement();
//     }
//
//     void releaseFeedbackChanged() {
//         fbReleaseAmp_.w = std::min(kMaxFeedback_, exp(-fbRelease_ * 10.0));
//     }
//
//     void decayModFrequencyChanged() {
//         phasorIncrement_.w = decayModFrequency_ / SampleRate::freq_;
//     }
//
//     /* Parameters */
//     Wire delayTime_;                // Delay: delay time in seconds
//     Wire attack_;                   // Attack: ?? to ??
//     Wire decay_;                    // Decay: 0.1 to 1.0
//     Wire decayModStart_;            // AttackEnergy: .001 to 1.8
//     Wire decayModEnd_;              // DecayEnergy: .010 to 3.0
//     Wire endingDecay_;              // Sustain: 0.0 to 1.4
//     Wire decayModCurve_;            // EnvelopeCurve: -1 to 1
//     Wire decayModFrequency_;        // EnvelopeFrequency: 0.0 to 100.0 [update with
//     decayModFrequencyChanged] Wire fbRelease_;                // Release: 0.0 to 1.0 [update with
//     releaseFeedbackChanged] Wire velocitySensitivity_{ 0.0 }; // 0.0 to 1.0 Wire velocity_{ 1.0 }; //
//     0.0 to 1.0
//
//     /* Coefficients */
//     Wire fbDecayAmp_;
//     Wire fbReleaseAmp_;
//     Wire phasorIncrement_;
// #pragma endregion
//
//     bool isIdle() {
//         return state_ == State::off;
//     }
//
//     void triggerAttack() {
//         peak_   = valFromVelocityAndSensitivity(velocity_, velocitySensitivity_);
//         phasor_ = 0.0;
//
//         // skip delay stage if delay time is negligible
//         if (delayTime_ >= SampleRate::period_) {
//             state_ = State::delay;
//         } else {
//             state_ = State::attack;
//         }
//     }
//
//     void reset() {
//         currentValue_  = 0.0;
//         state_         = State::off;
//         secondsPassed_ = 0.0;
//         phasor_        = 0.0;
//     }
//
//     void triggerRelease() {
//         state_ = State::release;
//         updateReleaseIncrement();
//     }
//
//     double run() {
//         switch (state_) {
//         case State::off:
//             break;
//         case State::delay:
//             secondsPassed_ += SampleRate::period_;
//             if (secondsPassed_ >= delayTime_) {
//                 state_ = State::attack;
//             }
//             break;
//         case State::attack:
//             updateAttackIncrement();
//             currentValue_ += attackIncrement_;
//             if (currentValue_ >= peak_) {
//                 state_        = State::decay;
//                 phasor_       = 0.0;
//                 currentValue_ = peak_;
//                 updateDecayIncrement();
//             }
//             break;
//         case State::decay:
//             fbDecayAmp_.w = calculateFbDecayAmp();
//             currentValue_ -= decayIncrement_ + currentValue_ * currentValue_ * fbDecayAmp_;
//             phasor_ += phasorIncrement_;
//             if (currentValue_ < 0.0) {
//                 reset();
//             }
//             break;
//         case State::release:
//             currentValue_ -= releaseIncrement_ + currentValue_ * currentValue_ * fbReleaseAmp_;
//             if (currentValue_ <= 0.0) {
//                 reset();
//             }
//             break;
//         }
//
//         return currentValue_;
//     }
//
//   private:
//     void updateAttackIncrement() {
//         const double attackTimeStart  = attack_;
//         const double attackTimeEnd    = 4.0 + attackTimeStart;
//         const double attackTimeLength = 1.0;
//         phasor_ += timeToIncrement(attackTimeLength, SampleRate::period_);
//         finalAttackMod_  = map0to1(std::min(phasor_, 1.0), attackTimeStart, attackTimeEnd);
//         attackIncrement_ = timeToIncrement(finalAttackMod_, SampleRate::period_);
//     }
//
//     double calculateFbDecayAmp() {
//         if (phasor_ < 1.0) {
//             finalDecayMod_ = decay_ + map0to1(exponentialCurve(phasor_, decayModCurve_), decayModStart_,
//             decayModEnd_);
//         } else {
//             finalDecayMod_ = endingDecay_;
//         }
//         return std::min(kMaxFeedback_, exp(-finalDecayMod_ * 10.0));
//     }
//
//     void updateDecayIncrement() {
//         decayIncrement_ = (currentValue_ - 1.0) * SampleRate::period_ / 50.0;
//     }
//     void updateReleaseIncrement() {
//         releaseIncrement_ = currentValue_ * SampleRate::period_ / 50.0;
//     }
//
//     State state_{};
//
//     double attackIncrement_{};
//     double decayIncrement_{};
//     double releaseIncrement_{};
//
//     double finalAttackMod_{};
//     double finalDecayMod_{};
//
//     double currentValue_{};
//     double secondsPassed_{};
//
//     double peak_{};
//     double phasor_{};
//
//     const double kMaxFeedback_ = 1.0 - 1.e-6; // std::nextafter(1.0, 0); // 0.999999;
// };
} // namespace soemdsp::modulator
