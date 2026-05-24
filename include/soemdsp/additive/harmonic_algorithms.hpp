#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <soemdsp/SampleRate.hpp>
#include <soemdsp/Wire.hpp>
#include <soemdsp/oscillator/PolyBLEP.hpp>
#include <soemdsp/sehelper.hpp>
#include <soemdsp/semath.hpp>

namespace soemdsp::additive {

class AdditiveProcessor {
  public:
    static void octavizer4(std::vector<double>& vec, int octaves) {
        std::vector<double> originalVector = vec;
        std::fill(vec.begin(), vec.end(), 0.0);

        for (size_t h = 1, i = 1; h < vec.size(); h *= octaves) {
            vec[h] = originalVector[h];
        }
    }

    // replace with a comb filter effect
    // static void squarizer(vector<double>& vec, int squares) {
    //    debug::CHECK(squares >= 2, "squares >= 2");
    //    for (size_t h = 0; h < vec.size(); h += squares) {
    //        vec[h] = 0.0;
    //    }
    //}

    static void spaceHarmonics(std::vector<double>& vec, int spaces) {
        debug::CHECK(spaces >= 2, "spaces >= 2");

        std::vector<double> originalVector = vec;
        std::fill(vec.begin(), vec.end(), 0.0);

        // for (size_t h = 1, i = 1; i < vec.size(); h += 1, i += spaces) {
        //     vec[i] = originalVector[h];
        // }

        // for (size_t h = 1, i = 1; i < vec.size(); h += 1, i += spaces) {
        //     vec[i] = originalVector[i];
        // }
    }

    // lame
    // static void rectify(vector<double>& amplitudes, vector<double>& phases, int spaces) {
    //    debug::CHECK(spaces >= 2, "spaces >= 2");

    //    vector<double> originalVector = amplitudes;
    //    std::fill(amplitudes.begin(), amplitudes.end(), 0.0);

    //    for (size_t h = 1, i = 1; i < amplitudes.size(); h += 1, i += spaces) {
    //        amplitudes[h] = originalVector[i];
    //    }
    //}

    static double wrapTAU(double phase) {
        phase -= constant::kTAU * floor(phase * constant::k1zTAU);
        return phase;
    }

    static double unitrisaw(double phase, double morph) {
        phase = wrapTAU(phase * constant::kTAU);
        morph *= constant::kTAU;

        double sourceRangeMin, sourceRangeMax, targetRangeMin, targetRange, sourceRange;

        if (phase > morph) {
            sourceRangeMin = morph;
            sourceRangeMax = constant::kTAU;
            targetRangeMin = 1;
            targetRange    = -1;
            sourceRange    = constant::kTAU - morph;
        } else {
            sourceRangeMin = 0;
            sourceRangeMax = morph;
            targetRangeMin = 0;
            targetRange    = 1;
            sourceRange    = morph - 0;
        }

        if (sourceRangeMin == sourceRangeMax) {
            return sourceRangeMin;
        }

        return targetRangeMin + (targetRange * (phase - sourceRangeMin)) / sourceRange;
    }

    static void phaseRotator(std::vector<double>& phasesMod, const double rotation, double length, const double curve, double distance, double morph) {
        double inc          = 0.5;
        double patternPhase = rotation;

        for (size_t i = 1; i < phasesMod.size(); ++i) {
            patternPhase += inc;
            patternPhase = math::wrap(patternPhase);
            phasesMod[i] = patternPhase;
        }

        // length = std::max(length, 1.0); // TODO: remove this line of code without crashing
        // debug::CHECK(length >= 1.0, "length >= 1.0");

        // double inc          = 1.0 / length;
        // double patternPhase = rotation;
        // double curveClamped = std::clamp(curve, -.999999, +.999999);

        // for (size_t i = 1; i < phasesMod.size(); ++i) {
        //     patternPhase += inc;
        //     patternPhase = wrap(patternPhase);

        //    double triangle = unitrisaw(patternPhase, morph);

        //    if (patternPhase >= 0.5) {
        //        phasesMod[i] = rationalCurve(triangle, curveClamped) * distance;
        //    } else {
        //        phasesMod[i] = rationalCurve(triangle, -curveClamped) * distance;
        //    }
        //}
    }
};
} // namespace soemdsp::additive
