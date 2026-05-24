#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <iterator>
#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
namespace soemdsp::debug {
#ifndef NDEBUG
inline void LOG(std::string_view msg, std::source_location loc = std::source_location::current()) {
    auto now        = std::chrono::system_clock::now();
    auto time_t_val = std::chrono::system_clock::to_time_t(now);

    //This buffer 'buf' is now correctly scoped within LOG
    char buf[32];

    //localtime is deprecated in some compilers; localtime_s or localtime_r is safer
    //Using simple ctime for minimal footprint:
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&time_t_val));

    std::string_view final_msg = msg.empty() ? "FAILURE: no error message provided" : msg;

    //Explicitly using fmt::print with a clear format string
    //Casting to std::string_view forces fmt to use the correct formatter
    fmt::print(stderr, "[{}] {}:{}:{} | {}\n", std::string_view(buf), std::string_view(loc.file_name()), std::string_view(loc.function_name()), loc.line(), final_msg);
}
inline void WARN(bool condition, std::string_view errormsg) {
    if (!condition) {
        LOG(errormsg);
    }
}
inline void CHECK(bool condition, std::string_view errormsg = "") {
    if (!condition) {
        LOG(errormsg);
        assert(false);
    }
}
inline void FAIL(std::string_view errormsg = "") {
    LOG(errormsg);
    assert(false);
}
inline void STOP(std::string_view msg = "DEBUG BREAK", std::source_location loc = std::source_location::current()) {
    LOG(msg, loc);
    assert(false);
}
template<typename Container> inline void WITHINSIZE(auto value, const Container& container, std::string_view msg = "Index out of bounds", std::source_location loc = std::source_location::current()) {
    if (static_cast<ptrdiff_t>(value) < 0 || static_cast<size_t>(value) >= container.size()) {
        LOG(msg, loc);
    }
}
template<typename t> void WITHINRANGE(t value, t min, t max) {
    CHECK(value >= min && value <= max);
}
//checks for bad value of: exploded (over 1 billion) / inf / nan / denormal
template<typename t> void BADVAL(t val) {
    CHECK(val < static_cast<t>(999999999), "number is exploded");
    CHECK(!std::isinf(val), "number is inf");
    CHECK(!std::isnan(val), "number is NaN");
    CHECK(std::fpclassify(val) != FP_SUBNORMAL, "number is denormalized");
}
#else
inline void LOG(std::string_view, std::source_location = {}) {}
inline void WARN(bool, std::string_view, std::source_location = {}) {}
inline void CHECK(bool, std::string_view = {}, std::source_location = {}) {}
inline void FAIL(std::string_view = "") {}
inline void STOP(std::string_view = {}, std::source_location = {}) {}
template<typename Container> inline void WITHINSIZE(auto, const Container&, std::string_view = {}, std::source_location = {}) {}
template<typename t> void WITHINRANGE(t, t, t) {}
template<typename t> void BADVAL(t val) {}
#endif
} //namespace soemdsp::debug

namespace soemdsp::compare {
static constexpr double COMPARE_ACCURACY = 1.e-7;
// clang-format off
inline bool is_equal (double a, double b) { return std::abs(a - b) < COMPARE_ACCURACY; }
inline bool not_equal(double a, double b) { return std::abs(a - b) >= COMPARE_ACCURACY; }
// clang-format on
} //namespace soemdsp::compare

namespace soemdsp::convert {
// clang-format off
[[nodiscard]] constexpr bool   inline double_to_bool  (double v) noexcept { return v >= 0.5; }
[[nodiscard]] constexpr int    inline double_to_int   (double v) noexcept { return static_cast<int>(v + 0.5); }
[[nodiscard]] std::string      inline double_to_string(double v) noexcept { return std::to_string(v); }
[[nodiscard]] constexpr size_t inline double_to_sizet (double v) noexcept { return static_cast<size_t>(v); }
template<typename E> [[nodiscard]] size_t                enumcount_to_sizet() noexcept { return magic_enum::enum_count<E>(); }
template<typename E> [[nodiscard]] constexpr size_t      enum_to_sizet (E e)  noexcept { return *magic_enum::enum_index<E>(e); }
template<typename E> [[nodiscard]] constexpr int         enum_to_int   (E e)  noexcept { return magic_enum::enum_integer<E>(e); }
template<typename E> [[nodiscard]] constexpr std::string enum_to_string(E e)  noexcept { return magic_enum::enum_name<E>(e); }
template<typename E> [[nodiscard]] constexpr double      enum_to_double(E e)  noexcept { return static_cast<double>(*magic_enum::enum_index<E>(e)); }
// clang-format on
template<typename E> [[nodiscard]] std::string_view int_to_enum_to_string(int v) noexcept {
    if (auto e = magic_enum::enum_cast<E>(v); e.has_value()) {
        return magic_enum::enum_name(e.value());
    }
    debug::FAIL();
    return magic_enum::enum_name(magic_enum::enum_values<E>()[0]);
}

template<typename E> [[nodiscard]] std::string_view double_to_enum_to_string(double v) noexcept {
    int index = static_cast<size_t>(v);
    if (auto e = magic_enum::enum_cast<E>(index); e.has_value()) {
        return magic_enum::enum_name(e.value());
    }
    debug::FAIL();
    return magic_enum::enum_name(magic_enum::enum_values<E>()[0]);
}
template <typename E> [[nodiscard]] E double_to_enum(double v) noexcept {
    int index = static_cast<int>(v);

    if (auto e = magic_enum::enum_cast<E>(index); e.has_value()) {
        return e.value();
    }

    debug::FAIL();
    return magic_enum::enum_values<E>()[0];
}
template<typename E> [[nodiscard]] E sizet_to_enum(size_t v) {
    auto values = magic_enum::enum_values<E>();

    if (v < values.size()) { return values[v]; }

    debug::FAIL();
    return values[0];
}
template<typename E> [[nodiscard]] E int_to_enum(int v) {
    auto values = magic_enum::enum_values<E>();

    if (v < values.size()) { return values[v]; }

    debug::FAIL();
    return values[0];
}

} //namespace soemdsp::convert
namespace soemdsp::global {
struct GlobalCounter {
    static int get() {
        static int counter = 0;
        return ++counter;
    }
    //typedef std::optional<std::reference_wrapper<size_t>> optional_ref_t;
    //static constexpr size_t MAX_OVERSAMPLES = 128;
};

} //namespace soemdsp::global
namespace soemdsp::op {
inline void empty() noexcept {}
} //namespace soemdsp::op
