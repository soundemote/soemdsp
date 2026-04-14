#pragma once

#include "semath.hpp"

#include <algorithm>
#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <functional>
#include <numbers>

using std::abs;
using std::floor;
using std::fmod;
using std::round;
using std::tuple;
using std::pow;
using std::log;
using std::log2;
using std::sin;
using std::cos;
using std::max;
using std::min;
using std::clamp;

namespace soemdsp {
double wrap(double phase) {
    return phase - floor(phase);
}

double wrap(double phase, double modulus) {
    double wrappedValue = fmod(phase, modulus);
    if (wrappedValue < 0) {
        wrappedValue += modulus;
    }
    return wrappedValue;
}

double sin(double phase) {
    return SINEWAVETABLE.cache_[size_t(wrap(phase) * SineWavetable::SIZE)];
}
double cos(double phase) {
    return SINEWAVETABLE.cache_[size_t(wrap(phase + 0.25) * SineWavetable::SIZE)];
}

bool updateInteger(int& i, double d) {
    int prevIntVal = i;
    i              = static_cast<int>(round(d));
    return prevIntVal != i;
}

tuple<double, double> sinCos(double phase) {
    return { sin(phase), cos(phase) };
}
void sinCos(double phase, double* sinOut, double* cosOut) {
    *sinOut = sin(phase);
    *cosOut = cos(phase);
}

double dbToAmp(double v) {
    return exp(v * 0.11512925464970228420089957273422);
}

double ampToDb(double v) {
    return 8.6858896380650365530225783783321 * log(v);
}

double primePower(int n) {
    return pow(PrimeTable[toSizeT(n)], .001);
}

double pitchToFrequency(double v) {
    return 8.1757989156437073336828122976033 * exp(0.057762265046662109118102676788181 * v);
}

double pitchToFrequencyNaive(double v) {
    return kMasterTune * pow(2.0, (v - 69.0) / 12.0);
}

double frequencyToPitch(double freq) {
    return 12.0 * log2(freq / kMasterTune) + 69.0;
}

bool isEven(int v) {
    return v % 2 == 0;
}

bool isOdd(int v) {
    return v % 2 == 1;
}

bool isOdd(size_t v) {
    return v % 2 == 1;
}

bool isNear(double a, double b, double epsilon = 1.e-7) {
    return abs(a - b) < epsilon;
}

double uniToBi(double v) {
    return v * 2.0 - 1.0;
}

double biToUni(double v) {
    return v * 0.5 + 0.5;
}

// 0 to 1 -> 0 to 1, 1 to 2 -> 1 to 0
double wrapBidirectional(double phase) {
    phase = abs(phase);

    if (static_cast<int>(phase) % 2 == 0) {
        return wrap(phase);
    } else {
        return 1 - wrap(phase);
    }
}

// 0 to .5 -> 0 to 1, .5 to 1 -> 1 to 0
double wrapBidirectional2(double phase) {
    return phase = 1.0 - abs((phase - floor(phase)) * 2 - 1);
}

// double triangle(double bipolarPhase) {
//     return 1.0 - 2.0 * abs(bipolarPhase - floor(bipolarPhase + 0.5));
// }
//
// double unipolarTriangle(double phase) {
//     if (phase < 0.5) {
//         return phase * 2.0;
//     } else {
//         return 2.0 - phase * 2.0;
//     }
// }

double positiveSlope(double v, double s) {
    if (s >= 0.0) {
        return s / (1.0 - s) * v;
    } else {
        return s;
    }
}

double negativeSlope(double v, double s) {
    return 1.0 - positiveSlope(v, s);
}

double negativeSlopeInv(double v, double s) {
    if (s >= 0.0) {
        const double Sd_Ss1 = s / (s - 1.0);
        return Sd_Ss1 * v - Sd_Ss1;
    } else {
        return s;
    }
}

double positiveSlopeInv(double v, double s) {
    return 1.0 - negativeSlopeInv(v, s);
}

double slope(double v, double x1, double y1, double x2, double y2) {
    return (y2 - y1) / (x2 - x1) * (v - x1) + y1;
}

double linearCrossfade(double a, double b, double crossfadeValue) {
    return (1.0 - crossfadeValue) * a + crossfadeValue * b;
}

void dualLinearCrossfade(double& a, double& b, double crossfadeValue) {
    a = (1.0 - crossfadeValue) * a;
    b = crossfadeValue * b;
}

void equalPowerCrossfade(double a, double b, double crossfadeValue, double* gainAOut, double* gainBOut) {
    *gainAOut = cos(crossfadeValue * kPIz2);
    *gainBOut = sin(crossfadeValue * kPIz2);
}

void stereoWidth(double width, double& l, double& r) {
    double widthInv = 1.0 - width;
    double m        = (l + r) * 0.5;
    l               = widthInv * m + width * l;
    r               = widthInv * m + width * r;
}

void stereoWidthEnhanced(double width, double& l, double& r) {
    //// calculate scale coefficient
    // double coef_S = width * 0.5;

    //// then do this per sample
    // double m = (l + r) * 0.5;
    // double s = (r - l) * coef_S;

    // l = m - s;
    // r = m + s;

    /* volume compensated */

    // calc coefs
    double tmp    = 1.0 / max(1.0 + width, 2.0);
    double coef_M = 1.0 * tmp;
    double coef_S = width * tmp;

    // then do this per sample
    double m = (l + r) * coef_M;
    double s = (r - l) * coef_S;

    l = m - s;
    r = m + s;
}

double valFromVelocityAndSensitivity(double velocity, double sensitivity) {
    return map0to1(velocity, 1.0 - sensitivity, 1.0);
}

int howManyHarmonics(double frequency, int userNumberHarmonics, double sampleRate, double maxFrequency, int maxHarmonics) {
    int n = static_cast<int>(clamp(min(sampleRate / frequency, maxFrequency), 0., toDouble(maxHarmonics)));
    return clamp(n, 0, userNumberHarmonics);
}
} // namespace soemdsp

namespace soemdsp::waveshape::unipolar {
double triangle(double phase) {
    return (phase < 0.5) ? (4 * phase - 1) : (3 - 4 * phase);
}
double trisaw(double phase, double warp) {
    // Precondition: 0 <= t <= 1 and 0 < warp < 1.
    return (phase < warp) ? (phase / warp) : ((1.0 - phase) / (1.0 - warp));
}
} // namespace soemdsp::waveshape::bipolar

namespace soemdsp::waveshape::bipolar {
double triangle(double phase) {
    return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
}
double trisaw(double phase, double warp) {
    // Precondition: 0 <= t <= 1 and 0 < warp < 1.
    return (phase < warp) ? (2.0 * (phase / warp) - 1.0) : (2.0 * ((1.0 - phase) / (1.0 - warp)) - 1.0);
}
} // namespace soemdsp::waveshape::bipolar
