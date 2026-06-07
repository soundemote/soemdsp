#pragma once

#include <algorithm>
#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <functional>
#include <numbers>

#include <soemdsp/semath.hpp>

using std::abs;
using std::clamp;
using std::cos;
using std::floor;
using std::fmod;
using std::log;
using std::log2;
using std::max;
using std::min;
using std::pow;
using std::round;
using std::sin;
using std::tuple;

namespace soemdsp::math {


bool updateInteger(int& i, double d) {
    int prevIntVal = i;
    i              = static_cast<int>(round(d));
    return prevIntVal != i;
}


double primePower(int n) {
    return pow(constant::PrimeTable[static_cast<size_t>(n)], .001);
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

bool isNear(double a, double b, double epsilon) {
    return abs(a - b) < epsilon;
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
    *gainAOut = cos(crossfadeValue * constant::kPIz2);
    *gainBOut = sin(crossfadeValue * constant::kPIz2);
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
    int n = static_cast<int>(clamp(min(sampleRate / frequency, maxFrequency), 0., static_cast<double>(maxHarmonics)));
    return clamp(n, 0, userNumberHarmonics);
}
} // namespace soemdsp
