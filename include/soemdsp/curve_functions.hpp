#pragma once

#include "sehelper.hpp"
#include "semath.hpp"
#include "soemdsp/utility/Cache.hpp"

#include <algorithm>
#include <functional>

namespace soemdsp::curve {
double binaryInverse(std::function<double(double)> curve, double target, double minSearchRange, double maxSearchRange, double precision = 1.e-7, int maxIterations = 20);

template<typename curve_t>
double calculateSkew(double normalizedPosition, double desiredValueForPosition, double min = 0.0, double max = 1.0) {
    double desiredValueIn = min + normalizedPosition * (max - min);

    SE_ASSERT(std::clamp(normalizedPosition, 0.0, 1.0) == normalizedPosition);
    SE_ASSERT(std::clamp(desiredValueForPosition, std::min(min, max), std::max(min, max)) == desiredValueForPosition);

    auto f = [&](double skew) { return curve_t{ skew, min, max }.get(desiredValueIn); };

    return binaryInverse(f, desiredValueForPosition, -1.0, +1.0, 1.e-8, 50);
}

// similar to exponential curve with .99 for t
double linearToExponential(double x, double min, double max);
// similar to exponential curve with .99 for t
double exponentialToLinear(double x, double min, double max);

struct CurveBase {
    SE_DEFAULT_CONSTRUCTOR(CurveBase);

    virtual void setup(double skew = 0.0, double min = 0.0, double max = 1.0) {
        min_       = min;
        max_       = max;
        skew_      = skew;
        max_m_min_ = max - min;
    }

    virtual double get(double in) const {
        return in;
    }
    virtual double getInv(double in, double precision = 1.e-8, int maxIterations = 30) const {
        return binaryInverse([this](double v) { return get(v); }, in, min_, max_, precision, maxIterations);
    }

    virtual double getCached(double in) const {
        return cachedValues_.lerp(in);
    }
    virtual double getCachedInv(double in) const {
        return cachedValuesInverted_.lerp(in);
    }

    virtual void cacheValues(double maxError = 0.000001) {
        cachedValues_.resample(min_, max_, maxError, [this](double x) { return get(x); });
    }
    virtual void cacheValuesInverted(double maxError = 0.000001) {
        cachedValues_.resample(min_, max_, maxError, [this](double x) { return getInv(x, 1.e-8, 100); });
    }

  protected:
    double skew_{};
    double min_{};
    double max_{ 1.0 };
    double max_m_min_{ 1.0 };
    utility::AdaptiveCache<double> cachedValues_;
    utility::AdaptiveCache<double> cachedValuesInverted_;
};

struct Linear : public CurveBase {
    SE_CUSTOM_CONSTRUCTOR(Linear)(double skew = 0.0, double min = 0.0, double max = 1.0) {
        setup(skew, min, max);
    }
    virtual void setup(double skew, double min = 0.0, double max = 1.0) override {
        CurveBase::setup(skew, min, max);
    }
    double getInv(double in, double, int) const override {
        return in;
    }
    double getCached(double in) const override {
        return in;
    }
    double getCachedInv(double in) const override {
        return in;
    }
    void cacheValues(double) override {}
    void cacheValuesInverted(double) override {}
};

// for a plot, see: https://www.desmos.com/calculator/rlyj4l7ybd
// value is 0 to 1, tension is -1 to +1, negative tension is rise slow, positive is rise fast
struct Rational : public CurveBase {
    SE_CUSTOM_CONSTRUCTOR(Rational)(double skew = 0.0, double min = 0.0, double max = 1.0) {
        CurveBase::setup(skew, min, max);
    }
    double get(double in) const {
        const double t   = (in - min_) / max_m_min_;
        const double out = ((1.0 + skew_) * t) / (1.0 - skew_ + 2.0 * skew_ * t);
        return min_ + max_m_min_ * out;
    }
};

// for a plot, see: https://www.desmos.com/calculator/rlyj4l7ybd
// value is 0 to 1, tension is -1 to +1, negative tension is rise slow, positive is rise fast
struct Exponential : public CurveBase {
    SE_CUSTOM_CONSTRUCTOR(Exponential)(double skew = 0.0, double min = 0.0, double max = 1.0) {
        setup(skew, min, max);
    }

    double get(double in) const override {
        if (skew_ == 0.0) {
            return in;
        }

        double I = (in - min_) / max_m_min_;
        return min_ + max_m_min_ * ((1.0 - exp(I * a_)) / _1_m_exp_a_);
    }

    void setup(double skew, double min = 0.0, double max = 0.0) {
        CurveBase::setup(skew, min, max);
        a_          = linVsExpFormulaScaler(skew_);
        _1_m_exp_a_ = 1.0 - exp(a_);
    }

  protected:
    inline double linVsExpFormulaScaler(double p) {
        const double c = 0.5 * (p + 1.0);
        return 2.0 * log10((1.0 - c) / c);
    }

    double a_{};
    double _1_m_exp_a_{};
};

// value is 0 to 1, tension is -1 to +1, negative tension is rise fast-slow-fast positive is rise slow-fast-slow
template<typename curve_t>
struct Sigmoid : public CurveBase {
    SE_CUSTOM_CONSTRUCTOR(Sigmoid)(double skew = 0.0, double min = 0.0, double max = 1.0) {
        halfWay_ = (max + min) * 0.5;
        lowCurve.setup(skew, min, halfWay_);
        highCurve.setup(-skew, halfWay_, max);
    }

    double get(double in) const {
        if (in < halfWay_) {
            return lowCurve.get(in);
        } else {
            return highCurve.get(in);
        }
    }

    void setup(double skew, double min = 0.0, double max = 1.0) {
        CurveBase::setup(skew, min, max);
        lowCurve.setup(skew, min, max);
        highCurve.setup(-skew, min, max);
    }

  protected:
    double halfWay_{};
    curve_t lowCurve;
    curve_t highCurve;
};

template <typename curve_t> struct Tilt {
    SE_DEFAULT_CONSTRUCTOR(Tilt);

    double get(double in, double tilt, double skew, double min, double max, double center) {
        if (skew_ != skew) {
            curve_.setup(skew);
            skew_ = skew;
        }

        SE_ASSERT(in >= 0.0 && in <= 1.0);
        SE_ASSERT(skew_ >= -1.0 && skew_ <= +1.0);
        SE_ASSERT(tilt_ >= -1.0 && tilt_ <= +1.0);

        tilt_ = tilt;
        min_  = min;
        max_ = max;
        center_ = center;

        fullvalue_ = map0to1(curve_.get(in), min_, max_);
        minTilt_   = mapNtoN(std::abs(tilt_), 0.0, 1.0, center_, min_);
        maxTilt_   = mapNtoN(std::abs(tilt_), 0.0, 1.0, center_, max_);

        if (tilt_ > 0.0) {
            return mapNtoN(fullvalue_, min_, max_, minTilt_, maxTilt_);
        } else {
            return mapNtoN(fullvalue_, min_, max_, maxTilt_, minTilt_);
        }
    }

    curve_t curve_;
    double skew_{};
    double tilt_{};
    double min_{};
    double max_{};
    double center_{};

    protected:
    // coefficients
    double fullvalue_{};
    double minTilt_{};
    double maxTilt_{};
};

} // namespace soemdsp::curve
