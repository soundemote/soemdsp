#include "MaxMSPHelper.h"

#include "soemdsp/sehelper.hpp"

using std::clamp;

namespace soemdsp::utility {

double gen::clip(double in, double min, double max) {
    return clamp(in, min, max);
}

double gen::sign(double in) {
    return soemdsp::sign(in);
}

double gen::phasor(double f) {
    // You forgot to call run_gen() or you didn't instantiate enough phasors!! DOOD. srrisly.
    SE_ASSERT(curPhasorIdx < phasors.size());

    phasors[curPhasorIdx].setFrequency(f);
    double ret = phasors[curPhasorIdx].getCurrentValue();
    ++curPhasorIdx;
    return ret;
}

void gen::run_gen() {
    for (auto& p : phasors)
        p.incrementSample();

    curPhasorIdx = 0;
    curChangeIdx = 0;
}

double gen::triangle(double x, double y) {
    return waveshape::unipolar::trisaw(wrap(x), y);
}

int gen::change(double v) {
    // You ran out of change() calls. How? You probably forgot to call run_gen(). Or you'll have to edit the class and increase the number of last_change_value indexes.
    SE_ASSERT(curChangeIdx < last_change_value.size());

    int ret = 0;

    if (last_change_value[curChangeIdx] > v)
        ret = 1;
    else if (last_change_value[curChangeIdx] < v)
        ret = -1;

    last_change_value[curChangeIdx] = v;

    ++curChangeIdx;

    return ret;
}
} // namespace soemdsp::utility
