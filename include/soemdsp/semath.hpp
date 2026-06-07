#pragma once

#include <algorithm>
#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <functional>
#include <numbers>
#include <string>
#include <string_view>

#include <soemdsp/seconstant.hpp>
#include <soemdsp/sehelper.hpp>

// calculate constants here : https://www.mathsisfun.com/scientific-calculator.html

namespace soemdsp::constant {
constexpr double kPI         = +3.1415926535897932385; // acos(-1)
constexpr double kA440       = +440.00000000000000000; // pitch of A3 or 69 (center frequency between minimum of 0 and maximum of 127)constexpr double kPI         = +3.14159265358979312; ///      acos(0)*2
constexpr double kPIz2       = +1.5707963267948966192; // acos(0)
constexpr double kPIz4       = +0.7853981633974483096; // acos(0)/2
constexpr double k2zPI       = +0.6366197723675813430; // 2/(acos(0)*2)
constexpr double k4zPI       = +1.2732395447351626862; // 4/(acos(0)*2)
constexpr double kTAU        = +6.2831853071795864769; // acos(0)*4
constexpr double k1zTAU      = +0.3183098861837906715; // 1/(acos(0)*2)
constexpr double ksin_PIx1p5 = -1.0000000000000000000; // sin(2*acos(0)*1.5)
constexpr double kcos_PIx1p5 = -0.0000000000000000000; // cos(2*acos(0)*1.5)
constexpr double k1zLN2      = +1.4426950408889634074; // 1/ln(2) or 1/log(2.0)
constexpr double k1zSQRT2    = +0.7071067811865475244; // 1/sqrt(2)
constexpr double kSQRT6x2    = +4.8989794855663561964; // sqrt(6) * 2
constexpr double kSQRT6z2    = +1.2247448713915890491; // sqrt(6) / 2
constexpr double kPHI        = +1.6180339887498948482; // Golden Ratio:          (sqrt(5)+1)/2
constexpr double k1zPHI      = +0.6180339887498948482; // Golden Ratio inverted: (sqrt(5)-1)/2

constexpr double k1z2      = 1.0 / 2.0;
constexpr double k1z3      = 1.0 / 3.0;
constexpr double k1z4      = 1.0 / 4.0;
constexpr double k1z5      = 1.0 / 5.0;
constexpr double k1z6      = 1.0 / 6.0;
constexpr double k1z11     = 1.0 / 11.0;
constexpr double k1z127    = 1.0 / 127.0;
constexpr double k1z180    = 1.0 / 180.0;
constexpr double kPIx1z180 = kPI * k1z180;
constexpr double k1z8192   = 1.0 / 8192.0;
constexpr double k1z16384  = 1.0 / 16384.0;
constexpr double k2z3      = 2.0 / 3.0;

constexpr size_t k2p15 = 32768; // 2^15
constexpr size_t k2p16 = 65536; // 2^16

constexpr std::array<int, 168> PrimeTable{
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139,
    149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
    347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769,
    773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997
};
} // namespace soemdsp::constant

namespace soemdsp::math {

inline double wrap(double phase) {
    return phase - floor(phase);
}

inline double wrap(double phase, double modulus) {
    double wrappedValue = fmod(phase, modulus);
    if (wrappedValue < 0) {
        wrappedValue += modulus;
    }
    return wrappedValue;
}


// updates 'i' based on rounded value 'd' and returns 'true' if 'i' was changed
inline bool updateInteger(int& i, double d);

constexpr size_t pow2(size_t exponent) {
    return 1ULL << exponent;
}

double primePower(int n);

template<typename t>
t map0to1(t v, t targetMin, t targetMax) {
    return v * (targetMax - targetMin) + targetMin;
}

template<typename t>
t mapNtoN(t v, t sourceMin, t sourceMax, t targetMin, t targetMax) {
    return targetMin + ((targetMax - targetMin) * (v - sourceMin)) / (sourceMax - sourceMin);
}

template<typename T>
double mapQuantize(T v, T sourceMin, T sourceMax, T targetMin, T targetMax) {
    if (std::abs(v - sourceMin) < std::abs(v - sourceMax)) {
        return targetMin; // Interpolated value if closer to the low bound
    } else {
        return targetMax; // Interpolated value if closer to the high bound
    }
}

bool isEven(int v);
bool isOdd(int v);
bool isOdd(size_t v);
bool isNear(double a, double b, double epsilon = constant::kPlanck);

// 0 to 1 -> 0 to 1, 1 to 2 -> 1 to 0
double wrapBidirectional(double phase);
// 0 to .5 -> 0 to 1, .5 to 1 -> 1 to 0
double wrapBidirectional2(double phase);
template<typename T>
T sign(T v) {
    return (T(0) < v) - (v < T(0));
}

inline double lerp(double value, double sourceLow, double sourceHigh, double destLow, double destHigh) {
    debug::CHECK(value >= sourceLow && value <= sourceHigh);
    double range           = sourceLow - sourceHigh;
    double distanceFromLow = value - sourceLow;
    double weight          = distanceFromLow / range;
    return destLow + weight * (destHigh - destLow);
}
template<typename t>
double lerp(double position, const std::vector<t>& vec) {
    int leftIndex  = static_cast<int>(position);
    int rightIndex = leftIndex + 1;

    debug::CHECK(rightIndex <= vec.size());

    if (rightIndex == vec.size()) {
        return vec.back();
    }

    double weight            = position - leftIndex;
    double interpolatedValue = vec[leftIndex] + weight * (vec[rightIndex] - vec[leftIndex]);
    return interpolatedValue;
}
// takes a 0 to 1 value and outputs a 0 to s value
double positiveSlope(double v, double s);
// takes a 0 to 1 value and outputs a 1 to s value
double negativeSlope(double v, double s);
// takes a 0 to 1 value and outputs an s to 0 value
double negativeSlopeInv(double v, double s);
// takes a 0 to 1 value and outputs an s to 1 value
double positiveSlopeInv(double v, double s);
double slope(double v, double x1, double y1, double x2, double y2);
// crossfades a and b values where fully a is a crossfade value of 0.
double linearCrossfade(double a, double b, double crossfadeValue);
// crossfades a and b values where fully a is a crossfade value of 0.
void equalPowerCrossfade(double a, double b, double crossfadeValue, double* gainAOut, double* gainBOut);
// adjust a and b based on crossfade value where fully a is a crossfade value of 0
void dualLinearCrossfade(double& a, double& b, double crossfadeValue);
// width value is 0.0 to 1.0
void stereoWidth(double width, double& l, double& r);
void stereoWidthEnhanced(double width, double& l, double& r);
// get a 0 to 1 final peak value with a combination of velocity and velocity sensitivity
double valFromVelocityAndSensitivity(double velocity, double sensitivity);
int howManyHarmonics(double frequency, int userNumberHarmonics, double sampleRate, double maxFrequency, int maxHarmonics);
template<typename container_t> double linearInterpolate(const container_t& values, double position) {
    if (position <= 0.0) {
        return values[0];
    }

    if (position >= 1.0) {
        return values.back();
    }

    double scaledPosition = position * static_cast<double>(values.size() - 1);
    size_t index          = static_cast<size_t>(scaledPosition);
    double fractionalPart = scaledPosition - index;

    return values[index] + fractionalPart * (values[index + 1] - values[index]);
}
template<typename container_t> double linearInterpolate(const container_t& values, double position, double min, double max) {
    double normPosition = (position - min) / (max - min);
    return linearInterpolate(values, normPosition);
}
template<typename t> double rounddouble(t v) {
    return round(static_cast<double>(v));
}
template<typename t> int roundint(t v) {
    return static_cast<int>(round(v));
}
template<typename t> uint8_t rounduint8(t v) {
    return static_cast<uint8_t>(round(v));
}
template<typename t> size_t roundsizet(t v) {
    return static_cast<size_t>(round(v));
}
} // namespace soemdsp::math

namespace soemdsp::convert {
constexpr double bipolar_to_radians(double v) {
    return v * constant::kPI;
}
inline double db_to_amp(double v) {
    return exp(v * 0.11512925464970228420089957273422);
}
inline double amp_to_db(double v) {
    return 8.6858896380650365530225783783321 * log(v);
}
inline double pitch_to_freq(double v) {
    return 8.1757989156437073336828122976033 * exp(0.057762265046662109118102676788181 * v);
}
inline double pitch_to_freq_naive(double v) {
    return constant::kA440 * pow(2.0, (v - 69.0) / 12.0);
}
inline double pitch_offset_to_freq_factor(double pitchOffset) {
    return exp(0.057762265046662109118102676788181 * pitchOffset);
    // return pow(2.0, pitchOffset/12.0); // naive, slower but numerically more precise
}
inline double freq_to_pitch(double freq) {
    return 12.0 * log2(freq / constant::kA440) + 69.0;
}
constexpr double to_bipolar(double v) {
    return v * 2.0 - 1.0;
}
constexpr double to_unipolar(double v) {
    return v * 0.5 + 0.5;
}
constexpr double degrees_to_phase(double degrees) {
    return degrees * constant::kPIx1z180;
}
} // namespace soemdsp::convert

namespace soemdsp::oscillator {

template<size_t TableSize = 1 << 16> // 65536
struct SineWavetable {
    static constexpr size_t SIZE = TableSize;

    SineWavetable() {
        for (size_t i = 0; i < SIZE; ++i) {
            double p  = static_cast<double>(i) / SIZE;
            array_[i] = std::sin(p * soemdsp::constant::kTAU);
        }
    }

    double sin(double phase) const noexcept {
        double s, c;
        sincos(phase, s, c);
        return s;
    }

    double cos(double phase) const noexcept {
        double s, c;
        sincos(phase, s, c);
        return c;
    }

    void sincos(double phase, double& s_out, double& c_out) const noexcept {
        phase = soemdsp::math::wrap(phase);

        size_t idx_sin = static_cast<size_t>(phase * SIZE) & (SIZE - 1);
        size_t idx_cos = static_cast<size_t>((phase + 0.25) * SIZE) & (SIZE - 1);

        s_out = array_[idx_sin];
        c_out = array_[idx_cos];
    }

  private:
    std::array<double, SIZE> array_{};
};

} // namespace soemdsp::oscillator

namespace soemdsp::global {
// Global singleton instance
inline const soemdsp::oscillator::SineWavetable<>& sineWavetable = []()
  -> const soemdsp::oscillator::SineWavetable<>& {
    static const soemdsp::oscillator::SineWavetable<> table;
    return table;
}();
} // namespace soemdsp::global

namespace soemdsp::math {

// ====================== GLOBAL CONVENIENCE FUNCTIONS ======================

inline double sin(double phase) noexcept {
    return soemdsp::global::sineWavetable.sin(phase);
}

inline double cos(double phase) noexcept {
    return soemdsp::global::sineWavetable.cos(phase);
}

inline void sincos(double phase, double& s_out, double& c_out) noexcept {
    soemdsp::global::sineWavetable.sincos(phase, s_out, c_out);
}
} // namespace soemdsp::math
