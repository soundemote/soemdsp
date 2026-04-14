#pragma once

#include "sehelper.hpp"

#include <algorithm>
#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <functional>
#include <numbers>

namespace soemdsp {

// calculate constants here: https://www.mathsisfun.com/scientific-calculator.html

/*
 * k = constant
 * z = divide
 * x = multiply
 *
 */

constexpr double kMasterTune = 440.0; // pitch of A3 or 69 (center frequency between minimum of 0 and maximum of 127)

constexpr double kPI         = +3.14159265358979312; ///      acos(0)*2
constexpr double kPIz2       = +1.57079632679489656; ///      acos(0)
constexpr double kPIz4       = +0.78539816339744828; ///      acos(0)/2
constexpr double k2zPI       = +0.63661977236758138; /// 2 / (acos(0)*2)
constexpr double k4zPI       = +1.27323954473516276; /// 4 / (acos(0)*2)
constexpr double kTAU        = +6.28318530717958623; ///      acos(0)*4
constexpr double k1zTAU      = +0.31830988618379069; /// 1 / (acos(0)*2)
constexpr double ksin_PIx1p5 = -1.00000000000000000; /// sin(2 * acos(0) * 1.5)
constexpr double kcos_PIx1p5 = -0.00000000000000000; /// cos(2 * acos(0) * 1.5)
constexpr double k1zLN2      = +1.44269504088896339; /// 1 / log(2.0)
constexpr double k1zSQRT2    = +0.70710678118654752; /// 1 / sqrt(2)
constexpr double kSQRT6x2    = +4.89897948556635620; /// sqrt(6) * 2
constexpr double kSQRT6z2    = +1.22474487139158905; /// sqrt(6) / 2
constexpr double kPHI        = +1.61803398874989485; /// Golden Ratio:          (sqrt(5)+1)/2
constexpr double k1zPHI      = +0.61803398874989485; /// Golden Ratio inverted: (sqrt(5)-1)/2

// floating point numbers are stored in the balls.

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

constexpr double k2z3 = 2.0 / 3.0; /// 2.0 / 3.0

constexpr size_t k2p15 = 32768; /// 2^15
constexpr size_t k2p16 = 65536; /// 2^16

class SineWavetable {
  public:
    static const size_t SIZE{ 1 << 16 };
    std::array<double, SIZE> cache_;

    SineWavetable() {
        for (size_t i = 0; i < SIZE; ++i) {
            cache_[i] = std::sin(toDouble(i) / toDouble(SIZE) * kTAU);
        }
    }
};

double wrap(double phase);
double wrap(double phase, double modulus);

const SineWavetable SINEWAVETABLE;
double sin(double phase);
double cos(double phase);

// updates 'i' based on rounded value 'd' and returns 'true' if 'i' was changed
bool updateInteger(int& i, double d);

std::tuple<double, double> sinCos(double phase);
void sinCos(double phase, double* sinOut, double* cosOut);

double dbToAmp(double v);
double ampToDb(double v);

constexpr size_t pow2(size_t exponent) {
    return 1ULL << exponent;
}

constexpr std::array<int, 168> PrimeTable{
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137, 139,
    149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
    347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769,
    773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997
};

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

double pitchToFrequency(double v);

double pitchToFrequencyNaive(double v);

double frequencyToPitch(double freq);
bool isEven(int v);
bool isOdd(int v);
bool isOdd(size_t v);
bool isNear(double a, double b, double epsilon);

// 0 to 1 -> -1 to +1
double uniToBi(double v);

// -1 to +1 -> 0 to 1
double biToUni(double v);

// -360 to 360 => -1 to 1
constexpr double degreesToPhase(double degrees) {
    return degrees * kPIx1z180;
}
// 0 to 1 -> 0 to 1, 1 to 2 -> 1 to 0
double wrapBidirectional(double phase);
// 0 to .5 -> 0 to 1, .5 to 1 -> 1 to 0
double wrapBidirectional2(double phase);
double triangle(double bipolarPhase);
double unipolarTriangle(double phase);
template<typename T>
T sign(T v) {
    return (T(0) < v) - (v < T(0));
}

inline double lerp(double value, double sourceLow, double sourceHigh, double destLow, double destHigh) {
    SE_ASSERT(value >= sourceLow && value <= sourceHigh);
    double range           = sourceLow - sourceHigh;
    double distanceFromLow = value - sourceLow;
    double weight          = distanceFromLow / range;
    return destLow + weight * (destHigh - destLow);
}

template<typename t>
double lerp(double position, const std::vector<t>& vec) {
    int leftIndex  = toInt(position);
    int rightIndex = leftIndex + 1;

    SE_ASSERT(rightIndex <= vec.size());

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

template<typename container_t>
double linearInterpolate(const container_t& values, double position) {
    if (position <= 0.0) {
        return values[0];
    }

    if (position >= 1.0) {
        return values.back();
    }

    double scaledPosition = position * toDouble(values.size() - 1);
    size_t index          = static_cast<size_t>(scaledPosition);
    double fractionalPart = scaledPosition - index;

    return values[index] + fractionalPart * (values[index + 1] - values[index]);
}

template<typename container_t>
double linearInterpolate(const container_t& values, double position, double min, double max) {
    double normPosition = (position - min) / (max - min);
    return linearInterpolate(values, normPosition);
}

// Inserts random values into a container from 0.0 to 1.0 inclusive
template<typename Container>
void insertRandomNormalizedValues(Container& arr, size_t startIndex, size_t endIndex, std::function<double()> randomFunction) {
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = randomFunction();
    }

    auto minmaxPair  = std::minmax_element(arr.begin() + startIndex, arr.begin() + endIndex);
    double randomMin = *minmaxPair.first;
    double randomMax = *minmaxPair.second;

    double scale{};
    double offset{ 1.0 };

    if (randomMax != randomMin) {
        scale  = 1.0 / (randomMax - randomMin);
        offset = -randomMin * scale;
    }
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = *it * scale + offset;
    }
}

// Inserts random values into a container from minValue to maxValue inclusive
template<typename Container>
void insertRandomValues(Container& arr, size_t startIndex, size_t endIndex, double minValue, double maxValue, std::function<double()> randomFunction) {
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = randomFunction();
    }

    auto minmaxPair = std::minmax_element(arr.begin() + startIndex, arr.begin() + endIndex);
    double randomMin   = *minmaxPair.first;
    double randomMax   = *minmaxPair.second;

    double scale{};
    double offset{ maxValue };
    
    if (randomMax != randomMin) { // if range is not degenerate
        scale  = (maxValue - minValue) / (randomMax - randomMin);
        offset = minValue - randomMin * scale;
    }
    for (auto it = arr.begin() + startIndex; it != arr.begin() + endIndex; ++it) {
        *it = *it * scale + offset;
    }
}

} // namespace soemdsp

namespace soemdsp::waveshape::unipolar {
double triangle(double phase);
double trisaw(double phase, double warp);
} // namespace soemdsp::waveshape::unipolar

namespace soemdsp::waveshape::bipolar {
double triangle(double phase);
double trisaw(double phase, double warp);
} // namespace soemdsp::waveshape::bipolar



