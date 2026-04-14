#include "curve_functions.hpp"

using std::function;

namespace soemdsp::curve {
double binaryInverse(function<double(double)> curve, double target, double minSearchRange, double maxSearchRange, double precision, int maxIterations) {
    double s = minSearchRange;
    double e = maxSearchRange;
    int i{ 0 };

    while (fabs(s - e) > precision && i++ < maxIterations) {
        auto cm = curve((s + e) * 0.5);
        if (cm > target) {
            e = (s + e) * 0.5;
        } else {
            s = (s + e) * 0.5;
        }
    }

    SE_ASSERT((i - 1) <= maxIterations);

    return (s + e) * 0.5;
};

// similar to exponential curve with .99 for t
double linearToExponential(double x, double min, double max) {
    const double coeff = log(max / min) / (max - min);
    return min * exp(coeff * (x - min));
}

// similar to exponential curve with .99 for t
double exponentialToLinear(double x, double min, double max) {
    const double coeff = log(max / min) / (max - min);
    return log(x / min) / coeff + min;
}
} // namespace soemdsp::curve
