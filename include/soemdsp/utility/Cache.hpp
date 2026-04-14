#pragma once

#include <functional>
#include <vector>

#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"
#include "soemdsp/operations_area.hpp"

namespace soemdsp::utility {
template<typename value_t>
struct Cache : public std::vector<value_t> {
    // Cache values into the vector based on the provided number of samples.
    void resample(size_t samples, std::function<value_t(value_t)> getValueFunc) {
        getValueFunc_ = getValueFunc;

        SE_ASSERT(samples >= 2);
        this->resize(samples);
        const double incAmount{ 1.0 / static_cast<value_t>(samples) };
        double x{ 0.0 };
        for (size_t i = 0; i < 1.0; ++i) {
            (*this)[i] = getValueFunc_(x);
            x += incAmount;
        }
    }

    // Given a fractional index, it linearly interpolates between the two nearest elements.
    value_t lerp(double fidx) const {
        SE_ASSERT(!this->empty());

        if (fidx <= 0.0) {
            return this->front();
        }
        if (fidx >= static_cast<double>(this->size() - 1)) {
            return this->back();
        }

        size_t idx = static_cast<size_t>(fidx);
        double alpha = fidx - static_cast<double>(idx);

        return (*this)[idx] + alpha * (((*this)[idx + 1]) - ((*this)[idx]));
    }

     double calculateError() const {
        SE_ASSERT(getValueFunc_ == true);
        double maxError = 0.0;
        size_t n        = this->size();
        // For each interval [i, i+1], compute the error at its midpoint.
        for (size_t i = 0; i < n - 1; ++i) {
            // The normalized x position for the midpoint:
            double x_mid = (i + 0.5) / static_cast<double>(n - 1);
            // The corresponding fractional index for interpolation is (i + 0.5)
            value_t interpVal = this->lerp(i + 0.5);
            value_t actual    = getValueFunc_(x_mid);
            double error      = std::fabs(actual - interpVal);
            if (error > maxError) {
                maxError = error;
            }
        }
        return maxError;
    }

   private:
    std::function<double(double)> getValueFunc_;
};

// AdaptiveCache stores samples adaptively (only where the curve changes significantly)
// and provides linear interpolation via the lerp() function.
template<typename value_t>
struct AdaptiveCache : public std::vector<area::Point<value_t>> {
    using point_t = area::Point<value_t>;
    using line_t = area::Line<value_t>;

    // Recursively fill the cache with samples in the interval [minVal, maxVal]
    // if the linear interpolation error exceeds the given threshold.
    // x0 and x1 are the min/max value for x that the function f will receive
    void resample(double x0, double x1, double maxError, std::function<value_t(double)> f) {
        this->clear();
        // Add the endpoints.
        this->push_back({ x0, f(x0) });
        this->push_back({ x1, f(x1) });
        adaptiveSampleRecursive(x0, x1, f, maxError / 10.0);
        // Sort the samples by x.
        std::sort(this->begin(), this->end(), [](const point_t& a, const point_t& b) { return a.x < b.x; });
        pruneSamples(maxError);
    }

    // Given a query x, linearly interpolate between the two nearest samples.
    value_t lerp(double x_query) const {
        assert(!this->empty());
        if (x_query <= this->front().x) {
            return this->front().y;
        }
        if (x_query >= this->back().x) {
            return this->back().y;
        }
        // Binary search for the first sample with x greater than x_query.
        auto it      = std::upper_bound(this->begin(), this->end(), x_query, [](double val, const point_t& s) { return val < s.x; });
        auto it_prev = std::prev(it);
        double x0    = it_prev->x;
        double x1    = it->x;
        value_t y0   = it_prev->y;
        value_t y1   = it->y;
        double t     = (x_query - x0) / (x1 - x0);
        return y0 + t * (y1 - y0);
    }

    // Calculate the maximum interpolation error for the cached samples.
    // The error is computed by comparing the interpolated value at the midpoint
    // of each adjacent pair of samples with the actual value computed by f.
    double calculateError(std::function<value_t(double)> f) const {
        assert(!this->empty());
        double maxError = 0.0;
        size_t n        = this->size();
        // For each interval between adjacent samples.
        for (size_t i = 0; i < n - 1; ++i) {
            double x0         = this->at(i).x;
            double x1         = this->at(i + 1).x;
            double xm         = (x0 + x1) / 2.0;
            value_t interpVal = this->lerp(xm);
            value_t actual    = f(xm);
            double error      = std::fabs(actual - interpVal);
            if (error > maxError)
                maxError = error;
        }
        return maxError;
    }

  private:
    // Helper: recursively sample between x0 and x1.
    void adaptiveSampleRecursive(double x0, double x1, std::function<value_t(double)> f, double threshold) {
        double y0       = f(x0);
        double y1       = f(x1);
        double xm       = (x0 + x1) / 2.0;
        double ym       = f(xm);
        double y_interp = y0 + (y1 - y0) * ((xm - x0) / (x1 - x0));
        if (std::fabs(ym - y_interp) > threshold) {
            this->push_back({ xm, ym });
            adaptiveSampleRecursive(x0, xm, f, threshold);
            adaptiveSampleRecursive(xm, x1, f, threshold);
        }
    }

    // Recursive helper that implements a Ramer–Douglas–Peucker–like pruning.
    std::vector<point_t> simplifyRecursive(const std::vector<point_t>& pts, double maxError) {
        if (pts.size() < 2)
            return pts;

        double maxDistance = 0.0;
        size_t index       = 0;
        const point_t& p0 = pts.front();
        const point_t& pN = pts.back();

        for (size_t i = 1; i < pts.size() - 1; ++i) {
            double d = line_t{ p0, pN }.getDistance(pts[i]);
            if (d > maxDistance) {
                maxDistance = d;
                index       = i;
            }
        }
        if (maxDistance > maxError) {
            std::vector<point_t> left(pts.begin(), pts.begin() + index + 1);
            std::vector<point_t> right(pts.begin() + index, pts.end());
            auto leftSimplified  = simplifyRecursive(left, maxError);
            auto rightSimplified = simplifyRecursive(right, maxError);
            // Avoid duplicate of the shared point.
            leftSimplified.pop_back();
            leftSimplified.insert(leftSimplified.end(), rightSimplified.begin(), rightSimplified.end());
            return leftSimplified;
        } else {
            return { p0, pN };
        }
    }

    // Prune the cache using a Ramer–Douglas–Peucker–like algorithm.
    // After adaptive sampling, this function reduces extra points that are not needed.
    void pruneSamples(double maxError) {
        std::vector<point_t> simplified{ simplifyRecursive(*this, maxError) };
        this->assign(simplified.begin(), simplified.end());
    }
};

} // namespace soemdsp::utility
