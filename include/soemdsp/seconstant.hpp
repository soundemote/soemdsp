#pragma once

// Universe laws + tiny helpers. Compile-time only — never assign at runtime.
// kPI / kTAU and the rest of soemdsp::constant stay in semath.hpp.

namespace soemdsp::constant {
/// Universe tick (Planck). 1.0 is home. First real step is 1 + kPlanck.
constexpr double kPlanck = 1.e-7;
} // namespace soemdsp::constant

namespace soemdsp::math {
constexpr double planck() noexcept {
    return constant::kPlanck;
}

constexpr double unityAbs(double v) noexcept {
    return v < 0.0 ? -v : v;
}

/// True when peak is at least one Planck step over +1.
constexpr bool aboveUnity(double peak) noexcept {
    return peak >= 1.0 + constant::kPlanck;
}

/// True when |value| is at least one Planck step over unity.
constexpr bool outsideUnity(double value) noexcept {
    return aboveUnity(unityAbs(value));
}
} // namespace soemdsp::math
