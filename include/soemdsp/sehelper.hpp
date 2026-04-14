#pragma once

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <iostream>

struct GlobalCounter {
    static int get() {
        static int counter = 0;
        return ++counter;
    }
};

namespace soemdsp {
typedef std::optional<std::reference_wrapper<size_t>> optional_ref_t;
constexpr size_t MAX_OVERSAMPLES = 128;
} // namespace soemdsp

#define SE_DEFAULT_CONSTRUCTOR(className)                                                                                                                                                              \
    className(const className&)            = default;                                                                                                                                                  \
    className(className&&)                 = default;                                                                                                                                                  \
    virtual ~className()                   = default;                                                                                                                                                  \
    className& operator=(const className&) = default;                                                                                                                                                  \
    className& operator=(className&&)      = default;                                                                                                                                                  \
    className()                            = default;

#define SE_CUSTOM_CONSTRUCTOR(className)                                                                                                                                                               \
    className(const className&)            = default;                                                                                                                                                  \
    className(className&&)                 = default;                                                                                                                                                  \
    virtual ~className()                   = default;                                                                                                                                                  \
    className& operator=(const className&) = default;                                                                                                                                                  \
    className& operator=(className&&)      = default;                                                                                                                                                  \
    className

inline void se_log(std::string_view /*errormsg*/, std::source_location /*loc*/ = std::source_location::current()) {
    // fmt::format("{} {}:{}\smoother_t{}", loc.file_name(), loc.function_name(), loc.line(), errormsg);
}
inline void se_warn(bool check, std::string_view errormsg) {
    if (!check) {
        se_log(errormsg);
    }
}
inline void se_assert(bool check) {
    if (!check) {
        assert(false);
    }
}
inline void se_assert_log(bool check, std::string_view errormsg) {
    if (!check) {
        se_log(errormsg);
        assert(false);
    }
}
template <typename t> void se_badval(t val) {
    se_assert_log(val < static_cast<t>(1'000'000'000), "number is exploded");
    se_assert_log(!std::isinf(val), "number is inf");
    se_assert_log(!std::isnan(val), "number is NaN");
    se_assert_log(std::fpclassify(val) != FP_SUBNORMAL, "number is denormalized");
}

#ifdef NDEBUG
#    define SE_BREAK
#    define SE_WARN(test, msg)
#    define SE_ASSERT(test)
#    define SE_FAIL(msg)
#    define SE_ERROR(test, msg)
#    define SE_SWITCH_STATEMENT_FAILURE                                                                                                                                                                \
    default:                                                                                                                                                                                           \
        break;
#    define SE_BADVAL(test)
#    define SE_WITHIN_SIZE(value, container)
#    define SE_WITHIN_RANGE(value, min, max)
#else
#    define SE_BREAK se_assert_log(false, "DEBUG BREAK");
#    define SE_WARN(test, msg) se_warn(test, msg);
#    define SE_ASSERT(test) se_assert(test);
#    define SE_FAIL(msg) se_assert_log(false, msg);
#    define SE_ERROR(test, msg) se_assert_log(test, msg);
#    define SE_SWITCH_STATEMENT_FAILURE                                                                                                                                                                \
    default:                                                                                                                                                                                           \
        se_assert_log(false, "SWITCH STATEMENT FAILURE");                                                                                                                                              \
        break;
#    define SE_BADVAL(test) se_badval(test);
#    define SE_WITHIN_SIZE(value, container) se_assert(static_cast<ptrdiff_t>(value) >= 0 && static_cast<size_t>(value) < container.size());
#    define SE_WITHIN_RANGE(value, min, max) se_assert(value >= min && value <= max);
#endif

inline bool toBool(double v, double threshold = 0.5) {
    return v >= threshold;
}
inline bool toBool(int v) {
    return v != 0;
}
inline bool toBool(float v, float threshold = 0.5) {
    return v >= threshold;
}

template<typename t>
float toFloat(t v) {
    return static_cast<float>(v);
}

template<typename t>
constexpr double toDouble(t v) {
    return static_cast<double>(v);
}

template<typename t>
int toInt(t v) {
    return static_cast<int>(v);
}

template<typename t>
int roundToInt(t v) {
    return static_cast<int>(round(v));
}

template<typename t>
uint8_t roundToUInt8(t v) {
    return static_cast<uint8_t>(round(v));
}

template<typename t>
constexpr size_t toSizeT(t v) {
    return static_cast<size_t>(v);
}

template<typename t>
size_t roundToSizeT(t v) {
    return static_cast<size_t>(round(v));
}

template<typename t>
double roundToDouble(t v) {
    return round(static_cast<double>(v));
}

template<typename value_t>
uint32_t toId(value_t v) {
    return static_cast<uint32_t>(v);
}

namespace Enum {

// enum to integer
template<typename enum_t>
int i(enum_t e) {
    return magic_enum::enum_integer<enum_t>(e);
}

// enum to string
template<typename enum_t>
std::string_view str(int desiredIndex) {
    constexpr auto values = magic_enum::enum_values<enum_t>();
    SE_WITHIN_SIZE(desiredIndex, values);
    return magic_enum::enum_name(values[desiredIndex]);
}

// enum to sizeT
template<typename enum_t>
size_t sizeT(enum_t e) {
    return static_cast<size_t>(magic_enum::enum_integer<enum_t>(e));
}


// total enum entries
template<typename enum_t>
constexpr auto count() {
    return magic_enum::enum_count<enum_t>();
}


// highest value of enum entries
template<typename enum_t>
constexpr auto max() {
    return magic_enum::enum_underlying(magic_enum::enum_values<enum_t>()[magic_enum::enum_count<enum_t>() - 1]);
}

// lowest value of enum entries
template<typename enum_t>
constexpr auto min() {
    return magic_enum::enum_underlying(magic_enum::enum_values<enum_t>()[0]);
}

// return list of values for enum entries
template<typename enum_t>
constexpr auto list() {
    return magic_enum::enum_values<enum_t>();
}

// given a value, return the corresponding enum type
template<typename enum_t, typename value_t>
enum_t get(value_t value) {
    auto optional = magic_enum::enum_cast<enum_t>(value);
    SE_ASSERT(optional.has_value());
    return optional.value();
}

template<typename enum_t, typename value_t>
enum_t get(value_t value, enum_t defaultEnum) {
    return magic_enum::enum_cast<enum_t>(value).value_or(defaultEnum);
}

} // namespace Enum

inline int stringToInt(std::string_view v) {
    try {
        return std::stoi(v.data());
    } catch (std::invalid_argument&) {
        return {};
    }
}

inline double stringToDouble(std::string_view v) {
    try {
        return std::stod(std::string(v));
    } catch (std::invalid_argument&) {
        return {};
    }
}
