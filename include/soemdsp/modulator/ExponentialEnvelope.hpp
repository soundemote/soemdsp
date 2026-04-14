// http://www.earlevel.com/main/2013/06/01/envelope-generators/f

#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/semath.hpp"

namespace soemdsp::modulator {

class ExponentialDADSR {
  public:
    enum class State {
        off,
        delay,
        attack,
        decay,
        sustain,
        release
    };

    ExponentialDADSR() {
        reset();

        attackTimeChanged();
        decayTimeChanged();
        releaseTimeChanged();

        setAttackShape(0.3);
        setReleaseShape(0.0001);
    }

#pragma region UPDATE
    void attackTimeChanged() {
        attackRate = SampleRate::timeToSamples(attackTime_);
        attackCoef = calcCoef(attackRate, attackShape);
        attackBase = (1.0 + attackShape) * (1.0 - attackCoef);
    }
    void decayTimeChanged() {
        decayRate = SampleRate::timeToSamples(decayTime_);
        decayCoef = calcCoef(decayRate, releaseShape);
        sustainAmplitudeChanged();
    }
    void sustainAmplitudeChanged() {
        decayBase = (sustainAmp_ - releaseShape) * (1.0 - decayCoef);
    }
    void releaseTimeChanged() {
        releaseRate = SampleRate::timeToSamples(releaseTime_);
        releaseCoef = calcCoef(releaseRate, releaseShape);
        releaseBase = -releaseShape * (1.0 - releaseCoef);
    }

    /* Paremeters */
    Wire<double> delayTime_;
    Wire<double> attackTime_{ 0.5 };
    Wire<double> decayTime_{ 0.5 };
    Wire<double> sustainAmp_{ 0.5 };
    Wire<double> releaseTime_{ 0.5 };
    Wire<double> attackShape{ .3 };     // .0001 to 100
    Wire<double> releaseShape{ .0001 }; // .0001 to 100
    Wire<int> isLooping_;

    /* Outputs */
    double out_;
#pragma endregion

    void sampleRateChanged() {
        attackTimeChanged();
        decayTimeChanged();
        releaseTimeChanged();
    }

    bool isIdle() {
        return state_ == State::off;
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

    void triggerDecay() {
        out_   = sustainAmp_;
        state_ = State::sustain;
    }

    void triggerRelease() {
        state_ = State::release;
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
            out_ = attackBase + out_ * attackCoef;
            if (out_ >= 1.0) {
                out_   = 1.0;
                state_ = State::decay;
            }
            break;
        case State::decay:
            out_ = decayBase + out_ * decayCoef;
            if (out_ <= sustainAmp_) {
                triggerDecay();
            }
            break;
        case State::sustain:
            if (isLooping_ == 1) {
                state_ = State::attack;
            }
            break;
        case State::release:
            out_ = releaseBase + out_ * releaseCoef;
            if (out_ <= 0.0) {
                out_   = 0.0;
                state_ = State::off;
            }
        }
        return out_;
    }

    void reset() {
        state_         = State::off;
        secondsPassed_ = 0.0;
        out_           = 0.0;
    }

    void setAttackShape(double targetRatio) {
        if (targetRatio < 0.000000001)
            targetRatio = 0.000000001; // -180 dB
        attackShape = targetRatio;
        attackCoef  = calcCoef(attackRate, attackShape);
        attackBase  = (1.0 + attackShape) * (1.0 - attackCoef);
    }

    void setReleaseShape(double targetRatio) {
        if (targetRatio < 0.000000001)
            targetRatio = 0.000000001; // -180 dB
        releaseShape = targetRatio;
        decayCoef    = calcCoef(decayRate, releaseShape);
        releaseCoef  = calcCoef(releaseRate, releaseShape);
        sustainAmplitudeChanged();
        releaseBase = -releaseShape * (1.0 - releaseCoef);
    }

  protected:
    State state_;
    double peak_{};
    double attackRate{};
    double decayRate{};
    double releaseRate{};
    double attackCoef{};
    double decayCoef{};
    double releaseCoef{};
    double attackBase{};
    double decayBase{};
    double releaseBase{};
    double secondsPassed_{};

    double calcCoef(double rate, double targetRatio) {
        return (rate <= 0) ? 0.0 : std::exp(-std::log((1.0 + targetRatio) / targetRatio) / rate);
    }
};

class ExponentialDASR {
  public:
    enum class State {
        off,
        delay,
        attack,
        sustain,
        release
    };

    ExponentialDASR() {
        reset();

        attackTimeChanged();
        releaseTimeChanged();

        setAttackShape(0.3);
        setReleaseShape(0.0001);
    }

#pragma region UPDATE
    void attackTimeChanged() {
        attackRate = SampleRate::timeToSamples(attackTime_);
        attackCoef = calcCoef(attackRate, attackShape);
        attackBase = (1.0 + attackShape) * (1.0 - attackCoef);
    }
    void sustainAmplitudeChanged() {
        releaseBase = (sustainAmp_ - releaseShape) * (1.0 - releaseCoef);
    }
    void releaseTimeChanged() {
        releaseRate = SampleRate::timeToSamples(releaseTime_);
        releaseCoef = calcCoef(releaseRate, releaseShape);
        releaseBase = -releaseShape * (1.0 - releaseCoef);
    }

    /* Paremeters */
    Wire<double> delayTime_;
    Wire<double> attackTime_{ 0.5 };
    Wire<double> sustainAmp_{ 0.5 };
    Wire<double> releaseTime_{ 0.5 };
    Wire<double> attackShape{ .3 };     // .0001 to 100
    Wire<double> releaseShape{ .0001 }; // .0001 to 100
    Wire<int> isLooping_;               // 0 or 1
    Wire<int> sustainOffOn_;            // 0 or 1

    /* Outputs */
    double out_;
#pragma endregion

    void sampleRateChanged() {
        attackTimeChanged();
        releaseTimeChanged();
    }

    bool isIdle() {
        return state_ == State::off;
    }

    void transferState(ExponentialDASR& env) {
        state_         = env.state_;
        peak_          = env.peak_;
        out_           = env.out_;
        secondsPassed_ = env.secondsPassed_;
        if (env.state_ == State::delay) {
            triggerAttack();
        } else if (env.state_ == State::attack) {
            triggerAttack();
        } else if (env.state_ == State::sustain) {
            triggerSustain();
        } else if (env.state_ == State::release) {
            triggerRelease();
        } else {
            reset();
        }
    }

    void triggerAttack() {
        // skip delay stage if delay time is negligible
        if (delayTime_ < SampleRate::period_) {
            //  skip attack stage if attack time is negliglbe
            if (attackTime_ <= SampleRate::period_) {
                state_ = State::sustain;
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

    void triggerSustain() {
        out_   = 1.0;
        state_ = State::sustain;
    }

    void triggerRelease() {
        state_ = State::release;
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
            out_ = attackBase + out_ * attackCoef;
            if (out_ >= 1.0) {
                triggerSustain();
            }
            break;
        case State::sustain:
            if (sustainOffOn_ == 0 || isLooping_) {
                state_ = State::release;
            }
            break;
        case State::release:
            out_ = releaseBase + out_ * releaseCoef;
            if (out_ <= 0.0) {
                out_ = 0.0;
                if (isLooping_) {
                    state_ = State::attack;
                } else {
                    reset();
                }
            }
        }
        return out_;
    }

    void reset() {
        state_         = State::off;
        secondsPassed_ = 0.0;
        out_           = 0.0;
    }

    void setAttackShape(double targetRatio) {
        if (targetRatio < 0.000000001)
            targetRatio = 0.000000001; // -180 dB
        attackShape = targetRatio;
        attackCoef  = calcCoef(attackRate, attackShape);
        attackBase  = (1.0 + attackShape) * (1.0 - attackCoef);
    }

    void setReleaseShape(double targetRatio) {
        if (targetRatio < 0.000000001)
            targetRatio = 0.000000001; // -180 dB
        releaseShape = targetRatio;
        releaseCoef  = calcCoef(releaseRate, releaseShape);
        sustainAmplitudeChanged();
        releaseBase = -releaseShape * (1.0 - releaseCoef);
    }

  protected:
    State state_;
    double peak_{};
    double attackRate{};
    double releaseRate{};
    double attackCoef{};
    double releaseCoef{};
    double attackBase{};
    double releaseBase{};
    double secondsPassed_{};

    double calcCoef(double rate, double targetRatio) {
        return (rate <= 0) ? 0.0 : exp(-log((1.0 + targetRatio) / targetRatio) / rate);
    }
};
class ExponentialDecayWIP {
  public:
    enum class State {
        off,
        delay,
        attack,
        decay,
        sustain,
        release
    };

    void reset() {
        state_ = State::off;
        out_   = 0.0;
    }

    bool isIdle() {
        return state_ == State::off;
        // return nearValue(out_, 0.0, 1.e-3);
    }

    void triggerAttack() {
        state_      = State::attack;
        levelBegin_ = out_ + .001;
        levelEnd_   = peak_;

        out_   = levelBegin_;
        coeff_ = (log(levelEnd_) - log(levelBegin_)) / (attackTime_ * SampleRate::freq_);
    }

    void triggerDecay() {
        state_      = State::decay;
        levelBegin_ = out_;
        levelEnd_   = sustainAmp_;

        coeff_ = (log(levelEnd_) - log(levelBegin_)) / (decayTime_ * SampleRate::freq_);
    }

    void triggerSustain() {
        state_ = State::sustain;
        out_   = sustainAmp_;
    }

    void triggerRelease() {
        state_        = State::release;
        levelBegin_.w = out_;
        levelEnd_.w   = 0.0 + .001;

        out_   = levelBegin_;
        coeff_ = (log(levelEnd_) - log(levelBegin_)) / (releaseTime_ * SampleRate::freq_);
    }

    double run() {
        switch (state_) {
        case State::off:
            break;

        case State::attack:
            out_ += coeff_ * out_;
            if (out_ >= peak_)
                triggerDecay();
            break;

        case State::decay:
            out_ += coeff_ * out_;
            if (isNear(out_, sustainAmp_, 1.e-3)) {
                triggerSustain();
            }
            break;

        case State::sustain:
            break;

        case State::release: {
            out_ += coeff_ * out_;
            if (isNear(out_, 0.0, 1.e-3)) {
                reset();
            }
        } break;
        }

        return out_;
    }

    double out_{};
    double coeff_{};
    Wire<double> peak_{ 1.0 };

    Wire<double> levelBegin_;
    Wire<double> levelEnd_;
    Wire<double> attackTime_;
    Wire<double> decayTime_;
    Wire<double> releaseTime_;
    Wire<double> sustainAmp_;
    State state_{};
};

} // namespace soemdsp::modulator
