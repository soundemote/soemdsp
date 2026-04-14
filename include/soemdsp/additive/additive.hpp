#pragma once

#include "soemdsp/SampleRate.hpp"
#include "soemdsp/Wire.hpp"
#include "soemdsp/curve_functions.hpp"
#include "soemdsp/oscillator/PolyBLEP.hpp"
#include "soemdsp/random/Random.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/utility/Cache.hpp"    

#include <magic_enum.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace soemdsp::additive {
class Harmonic {
  public:
    // waveform
    double waveformAmplitude_{ 1.0 };
    double waveformPhase_{};

    // frequency
    void increment() {
        phase_ = wrap(phase_ + increment_);
    }
    double pitch_{};
    double frequency_{};
    double increment_{};

    // phase
    double phase_{};

    // amplitude
    double leftAmp_{ 1.0 };
    double rightAmp_{ 1.0 };

    // panning
    void incrementPanner() {
        panRotationPhase_ = wrap(panRotationPhase_ + panRotationIncrement_);
    }
    double panRotationPhase_{};
    double panRotationIncrement_{};

    // filter
    double filterProfile_{};
    double filterAmpAdjust_{};
};

class AdditiveWaveform {
    friend class Additive;

  public:
    enum class Waveform {
        Sawtooth,
        SawSquare,
        DoubleSaw,
        MultiSaw,
        RoundedSquareDoubleSaw,
        SquareDoubleSaw,
        PulseCenter,
        PulseLeft,
        PulseRight,
        MultiPulse1,
        MultiPulse2,
        Square,
        TriSaw,
        Triangle,
        RectifiedSine,
        RectifiedSineTri,
        Organ,
    };

    AdditiveWaveform(std::vector<Harmonic>& harmonics, size_t& lastHarmonicToRender)
      : harmonics_(harmonics)
      , lastHarmonicToRender_(lastHarmonicToRender) {}

    void numHarmonicsChanged() {
        waveformChanged();
    }

    void waveformChanged() {
        counter_ = 1;

        double inc = 1.0 / toDouble(harmonics_.size());
        double pos = 1.0;

        for (size_t n = 1; n < lastHarmonicToRender_; ++n) {
            n_ = n;
            h_ = toDouble(n);

            switch (Enum::get<Waveform>(waveform_)) {
            case Waveform::Sawtooth:
                sawtooth();
                break;
            case Waveform::SawSquare:
                sawsquare(1.0 - mod1_);
                break;
            case Waveform::DoubleSaw:
                doublesaw(mod1_);
                break;
            case Waveform::MultiSaw:
                multisaw(mod1_);
                break;
            case Waveform::RoundedSquareDoubleSaw:
                roundedSquareDoubleSaw(mod1_);
                break;
            case Waveform::SquareDoubleSaw:
                squareDoubleSaw(mod1_);
                break;
            case Waveform::PulseCenter:
                pulseCenter(mod1_);
                break;
            case Waveform::PulseLeft:
                pulseLeft(mod1_);
                break;
            case Waveform::PulseRight:
                pulseRight(mod1_);
                break;
            case Waveform::MultiPulse1:
                multipulse1(mod1_);
                break;
            case Waveform::MultiPulse2:
                multipulse2(mod1_);
                break;
            case Waveform::Square:
                square(mod1_);
                break;
            case Waveform::TriSaw:
                trisaw(mod1_);
                break;
            case Waveform::Triangle:
                triangle();
                break;
            case Waveform::RectifiedSine:
                rectifiedSine(mod1_);
                break;
            case Waveform::RectifiedSineTri:
                rectifiedSineTri(mod1_);
                break;
            case Waveform::Organ:
                organ(static_cast<int>(map0to1<double>(mod1_, 2.0, 13.0)));
                break;
                SE_SWITCH_STATEMENT_FAILURE
            }

            //double adjustForSine = curve::Exponential::get(std::min(pos, 1.0), toSine_);

            harmonics_[n].waveformAmplitude_ = a_; // * adjustForSine;
            harmonics_[n].waveformPhase_     = p_;// + phaseRotation_;
            pos -= inc;
        }
    }

    Wire<int> waveform_; // waveformChanged()
    Wire<double> mod1_;  // 0 to 1, // waveformChanged()
    Wire<double> phaseRotation_;
    Wire<double> toSine_; // waveformChanged()

    static std::string getWaveshapeString(int waveform) {
        return std::string(magic_enum::enum_name(magic_enum::enum_cast<Waveform>(waveform).value_or(Waveform::Sawtooth)));
    }

    static std::string getMod1String(int waveform, double mod1) {
        switch (magic_enum::enum_cast<Waveform>(waveform).value_or(Waveform::Sawtooth)) {
        case Waveform::SawSquare:
            return fmt::format("mix: {:.2f}", mod1);
        case Waveform::DoubleSaw:
            return fmt::format("pwm: {:.2f}", mod1);
        case Waveform::PulseCenter:
            return fmt::format("pwm: {:.2f}", uniToBi(mod1));
        case Waveform::TriSaw:
            return fmt::format("peak: {:.2f}", uniToBi(mod1));
        case Waveform::Organ:
            return fmt::format("harm: {}", static_cast<int>(map0to1(mod1, 2.0, 13.0)));
        default:
            return fmt::format("mod: {:.2f}", mod1);
        }
    }

  private:
    size_t n_{};             // harmonic index
    double h_{}, a_{}, p_{}; // harmonic, amplitude, phase
    std::vector<Harmonic>& harmonics_;
    size_t& lastHarmonicToRender_;
    int counter_{ 1 };

    void rectifiedSine(double phaseRotate = 0.0) {
        phaseRotate *= 0.5;
        a_ = 1.0 / (h_ * h_);
        if (n_ % 2 == 1) {
            p_ = 0.25;
        } else {
            p_ = 0.75;
        }
        p_ += phaseRotate;
    }

    void rectifiedSineTri(double mod = 0.0) {
        double hh = h_ * h_;
        a_ = soemdsp::sin(hh * 0.25 + mod) / hh ;
        p_ = 0.25;
    }

    void roundedSquareDoubleSaw(double mod = 0.0) {
        mod       = 0.125 + 0.75 * mod; // range 0 to 1 -> 0.125 to .875
        double hh = h_ * h_;
        a_        = soemdsp::sin(hh * 0.25 + mod) / hh;
        p_        = 0.0;
    }

    void squareDoubleSaw(double mod = 0.0) {
        mod       = 0.125 + 0.75 * mod; // range 0 to 1 -> 0.125 to .875
        double hh = h_ * h_;
        a_        = soemdsp::sin(hh * 0.25 + mod) / h_;
        p_        = 0.0;
    }

    void sawtooth(double phaseRotate = 0.0) {
        phaseRotate = phaseRotate * 0.5;
        a_          = 1.0 / h_;
        p_          = isOdd(n_) ? p_ = 0.5 + phaseRotate : 0.0;
    }

    void doublesaw(double pwm = 0.0) {
        pwm *= 0.5;
        a_ = soemdsp::cos(h_ * pwm) / h_;
        p_ = 0.0;
    }

    void sawsquare(double mix = 0.0) {
        mix = 1.0 - mix;
        p_ = 0.0;
        if (n_ % 2 == 1) {
            a_ = 1.0 / h_;
        } else {
            a_ = 1.0 / h_ * mix;
        }
    }

    void square(double phaseRotate = 0.0) {
        phaseRotate *= 0.5;
        a_ = 0.0;
        p_ = phaseRotate;
        if (n_ % 2 == 1) {
            a_ = 1.0 / h_;
        }
    }

    void pulseCenter(double pwm = 0.5) {
        pwm *= 0.5;
        a_ = soemdsp::sin(h_ * pwm) / h_;
        p_ = 0.25;
    }

    void pulseLeft(double pwm = 0.5) {
        pwm *= 0.5;
        a_ = soemdsp::sin(h_ * pwm) / h_;
        p_ = h_ * pwm + 0.25;
    }

    void pulseRight(double pwm = 0.5) {
        pwm *= 0.5;
        a_ = soemdsp::sin(h_ * pwm) / h_;
        p_ = h_ * -pwm + 0.25;
    }

    void multipulse1(double pwm = 0.5) {
        pwm *= 0.5;
        double hh = h_ * h_;
        a_        = soemdsp::cos(hh * .45 + pwm) / h_;
        p_        = 0.0;
    }

    void multipulse2(double pwm = 0.5) {
        pwm *= 0.5;
        double hh = h_ * h_;
        a_        = soemdsp::cos(hh * .475 + pwm) / h_;
        p_        = 0.0;
    }

    void multisaw(double pwm = 0.5) {
        pwm *= 0.5;
        double hh = h_ * h_;
        a_        = soemdsp::cos(hh * 0.3 + pwm) / h_;
        p_        = 0.0;
    }

    void trisaw(double pwm = 0.5) {
        pwm = std::clamp<double>(pwm, 0.001, .999);
        a_  = soemdsp::sin(0.5 * h_ * pwm) / (pwm * (1.0 - pwm) * h_ * h_) * 0.2;
        p_ = 0.0;
    }

    void triangle(double phaseRotate = 0.0) {
        phaseRotate *= 0.5;
        a_ = 0.0;
        p_ = 0.5;
        if (n_ % 2 == 1) {
            a_ = 1.0 / (h_ * h_);
        }
        if (n_ % 4 == 1) {
            p_ = 0.0;
        }
        p_ += phaseRotate;
    }

    void organ(int octaves = 2) {
        a_ = 0;
        p_ = 0.0;
        if (n_ == toSizeT(counter_)) {
            a_ = 1.0 / h_;
            counter_ *= octaves;
        }
    }
};

struct AdditiveFilter {
    enum class Filter {
        Lowpass,
        Highpass,
    };

    enum class Curve {
        Linear,
        Rational,
        Sinusoidal,
    };

    static std::string_view getFilterString(int v) {
        return Enum::str<Filter>(v);
    }
    static std::string_view getCurveString(int v) {
        return Enum::str<Curve>(v);
    }

    void numHarmonicsChanged() {
        sampleRateChanged();
    }

    AdditiveFilter(std::vector<Harmonic>& harmonics, size_t& numHarmonicsToRender)
      : harmonics_(harmonics)
      , lastHarmonicToRender_(numHarmonicsToRender) {
        curveChanged();
    }

    void curveChanged() {
        switch (Enum::get<Curve>(curveType_)) {
        case Curve::Linear:
            cacheFilterCurve_.resample(2, [](double x) { return x; });
            break;
        case Curve::Rational: 
            cacheFilterCurve_.resample(CACHE_SZ, [this](double x) { return curve::Rational{ skew_ }.get(x); });
            break;
        case Curve::Sinusoidal:
            cacheFilterCurve_.resample(CACHE_SZ, [this](double x) {
                double v  = 1.0 - (0.5 + 0.5 * soemdsp::sin((x / 1.0 - 0.5) * 0.5));
                double cv = skew_ * v;
                return (cv + v) / (2.0 * cv - skew_ + 1.0);
            });
            break;
        }

        // reject adjustment
        for (size_t i = 0; i < cacheFilterCurve_.size(); ++i) {
            cacheFilterCurve_[i] = std::clamp(reject_ + ((1 - reject_) * cacheFilterCurve_[i]) / 1.0, 0.0, 1.0);
        }

        sampleRateChanged();
    }

    void sampleRateChanged() {
        double o  = offset_ * (1.0 - slope_ * 0.5);
        double x1 = map0to1<double>(slope_, 0.0, 0.5) + o;
        double x2 = map0to1<double>(slope_, 1.0, 0.5) + o;

        double size             = toDouble(lastHarmonicToRender_);
        size_t startingHarmonic = static_cast<size_t>(std::max(x1 * size, 0.0));
        size_t endingHarmonic   = static_cast<size_t>(std::max(x2 * size, 0.0));

        double pos{};
        double start{};
        double end{};
        double inc = 1.0 / toDouble(endingHarmonic - startingHarmonic);

        switch (magic_enum::enum_cast<Filter>(filterType_).value_or(Filter::Lowpass)) {
        case Filter::Lowpass:
            start = 1.0;
            end   = 0.0;
            pos   = start;
            for (size_t i = startingHarmonic; i < lastHarmonicToRender_ && i < endingHarmonic; ++i) {
                harmonics_[i].filterAmpAdjust_ = cacheFilterCurve_.lerp(pos);
                pos -= inc;
            }
            break;
        case Filter::Highpass:
            start = 0.0;
            end   = 1.0;
            pos   = start;
            for (size_t i = startingHarmonic; i < lastHarmonicToRender_ && i < endingHarmonic; ++i) {                
                harmonics_[i].filterAmpAdjust_ = cacheFilterCurve_.lerp(pos);
                pos += inc;
            }
            break;
        }
        for (size_t i = 0; i < startingHarmonic; ++i) {
            harmonics_[i].filterAmpAdjust_ = start;
        }
        for (size_t i = endingHarmonic; i < lastHarmonicToRender_; ++i) {
            harmonics_[i].filterAmpAdjust_ = end;
        }
    }

    Wire<int> curveType_{ [this]() { curveChanged(); } }; // 0 to Enum::count<Curve>();
    Wire<double> skew_{ [this]() { curveChanged(); } };   // -1 to +1
    Wire<double> reject_{ [this]() { curveChanged(); } }; // -1 to 0

    Wire<int> filterType_{ [this]() { sampleRateChanged(); } };
    Wire<double> slope_{ [this]() { sampleRateChanged(); } };   // 0 to 1
    Wire<double> offset_{ [this]() { sampleRateChanged(); } };  // -1 to +1

  protected:
    std::vector<Harmonic>& harmonics_;
    utility::Cache<double> cacheFilterCurve_;
    size_t& lastHarmonicToRender_;
    size_t CACHE_SZ = 1024;
};

class AdditivePanner {
    friend class Additive;

  public:
#pragma region update
    soemdsp::random::LCG32 random_;

    explicit AdditivePanner(std::vector<Harmonic>& harmonics)
      : harmonics_(harmonics) {}

    void rotationIncrementChanged(double minFreq, double maxFreq, double freqTiltOffset) {
        double inc = 1.0 / toDouble(harmonics_.size());
        double pos = 0;
        for (size_t n = 1; n < harmonics_.size(); ++n) {
            double f = curve::Tilt<curve::Rational>{}.get(pos, rotationIncrementTilt_, rotationIncrementTiltCurve_, minFreq, maxFreq, SampleRate::incrementToFrequency(rotationIncrement_));
            harmonics_[n].panRotationIncrement_ = SampleRate::frequencyToIncrement(f + freqTiltOffset);
            pos += inc;
        }
    }

    Wire<double> rotationIncrement_;
    Wire<double> rotationIncrementTilt_;
    Wire<double> rotationIncrementTiltCurve_;

    Wire<double> randomSpread_;
    Wire<double> alternatingSpread_;

    std::vector<double> randomValues_;
#pragma endregion
  private:
    std::vector<Harmonic>& harmonics_;

    void numHarmonicsChanged() {
        randomValues_.resize(harmonics_.size());
        for (auto& index : randomValues_) {
            index = random_.runBipolar();
        }
    }
    void start() {
        outL_ = 0;
        outR_ = 0;
    }

    void runHarmonic(size_t n) {
        auto& h = harmonics_[n];

        double panning = 0;

        if (n % 2 == 1) {
            panning += alternatingSpread_;
        } else {
            panning += -1.0 * alternatingSpread_;
        }

        h.incrementPanner();
        bipolarPhase_ = uniToBi(h.panRotationPhase_ + 0.5);

        panning = std::clamp(panning + randomValues_[n] * randomSpread_, -1.0, +1.0);
        panning = (triangle(panning + bipolarPhase_) - 0.5) * 2.0;

        // h.rightAmp_  = (1.0 - panning) * 0.5;
        // h.leftAmp_ = 1.0 - h.rightAmp_;
        h.rightAmp_ = abs(panning - 1.0);
        h.leftAmp_  = 2.0 - h.rightAmp_;
        h.rightAmp_ = clamp(h.rightAmp_, 0.0, 1.0);
        h.leftAmp_  = clamp(h.leftAmp_, 0.0, 1.0);
    }

    double outL_{};
    double outR_{};

    double bipolarPhase_{};
};

/*
 * Must provide a sineWaveTable pointer and AdditiveCache pointer.
 */
class Additive {
  public:
    Additive() {
        harmonics_.resize(2);
    }

#pragma region UPDATE
    void incrementChanged() {
        for (size_t n = 1; n < harmonics_.size(); ++n) {
            harmonics_[n].increment_ = increment_ * toDouble(n);
        }
        numHarmonicsChanged();
    }

    void numHarmonicsChanged() {
        harmonics_.resize(toSizeT(numHarmonics_) + 1);
        lastHarmonicToRender_ = toSizeT(std::min(MAX_FREQUENCY_ / SampleRate::incrementToFrequency(increment_), toDouble(harmonics_.size())));

        additivePanner_.numHarmonicsChanged();
        additiveWaveform_.waveformChanged();
        additiveFilter_.numHarmonicsChanged();
        additiveFilter_.sampleRateChanged();
    }

    inline void run();
    double outL_{};
    double outR_{};
    double amptemp_{ 1.0 };

    Wire<double> increment_{};
    Wire<int> numHarmonics_{ 1 };
    Wire<double> phaseSkew_;
    Wire<double> phaseSkewCurve_;
    std::vector<Harmonic> harmonics_;

    AdditiveWaveform additiveWaveform_{ harmonics_, lastHarmonicToRender_ };
    AdditivePanner additivePanner_{ harmonics_ };
    AdditiveFilter additiveFilter_{ harmonics_, lastHarmonicToRender_ };

#pragma endregion
    size_t lastHarmonicToRender_{ 1 };
    const double MAX_FREQUENCY_{ 20000.0 };

    void envReset() {}

    void oscReset() {}

    bool isIdle() {
        return amptemp_ >= 0.0;
    }

    void triggerAttack() {
        amptemp_ = 1.0;
        for (auto& h : harmonics_) {
            h.phase_ = 0.0;
        }
    }

    void triggerRelease() {
        amptemp_ = 0.0;
    }
};

inline void Additive::run() {
    //double skewRamp              = 0;
    //double phaseSkewCurveClamped = clamp<double>(phaseSkewCurve_, -.999999, +.999999);

    outL_ = 0.0;
    outR_ = 0.0;
    //for (size_t n = 1; n < harmonics_.size(); ++n) {
    //    harmonics_[n].increment();
    //    if (n > lastHarmonicToRender_) {
    //        continue;
    //    }

    //    /* skew phase */
    //      double skew = curve::RationalS::get(1.0 - skewRamp, phaseSkewCurveClamped) * 300.0;
    //    skewRamp    = wrap(skewRamp + phaseSkew_);
    //    /**************/

    //    double phase  = harmonics_[n].phase_ + harmonics_[n].waveformPhase_;
    //    double signal = soemdsp::sin(phase + skew);

    //    //signal *= harmonics_[n].waveformAmplitude_;
    //    //signal *= harmonics_[n].filterAmpAdjust_;

    //    // additivePanner_.runHarmonic(n);
    //    outL_ += signal; // * harmonics_[n].leftAmp_;
    //    //outR_ += signal; // * harmonics_[n].rightAmp_;
    //    outL_ = outR_;
    //}
    for (size_t n = 1; n < harmonics_.size(); ++n) {
        harmonics_[n].increment();
        if (n > lastHarmonicToRender_) {
            continue;
        }

        double signal = soemdsp::sin(harmonics_[n].phase_ + harmonics_[n].waveformPhase_) * harmonics_[n].waveformAmplitude_;

        outL_ += signal;
    }
    outR_ = -uniToBi(harmonics_[1].phase_);
}
} // namespace soemdsp::additive
