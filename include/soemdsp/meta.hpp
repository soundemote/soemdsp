#pragma once

#include <array>
#include <span>
#include <string_view>

namespace soemdsp::meta {

namespace choice {
static constexpr std::string_view waveform[]  = { "Sine", "Saw", "Square", "Noise" };
static constexpr std::string_view bypass[]    = { "active", "BYPASSED" };
static constexpr std::string_view plusminus[] = { "-", "+" };
static constexpr std::string_view onoff[]     = { "off", "on" };
static constexpr std::string_view momentary[] = { "idle", "on" };
} //namespace choice

enum class MetaType {
    //decimal
    decimal,
    decimal_bipolar,
    amplitude,
    decibels,
    frequency,
    pitch,
    seconds,
    sustain,

    //integer
    descrete,

    //choice
    waveform,

    //boolean
    bypass,
    plusminus,
    onoff,
    momentary,
};

struct WireTypeTraits {
    std::string_view const unit_{};
    const double def_{};
    const double min_{};
    const double max_{};
    std::span<const std::string_view> choice{};
    bool showPlusMinus{};

    static constexpr const WireTypeTraits get(MetaType t) noexcept {
        switch (t) {
        //decimal
        case MetaType::decimal:
            return { "lin", 0.0, 0.0, 1.0 };
        case MetaType::decimal_bipolar:
            return { "lin", 0.0, -1.0, 1.0, {}, true };
        case MetaType::amplitude:
            return { "amp", 1.0, 0.0, 3.0 };
        case MetaType::decibels:
            return { "dB", 0.0, -60.0, 12.0, {}, true };
        case MetaType::frequency:
            return { "Hz", 1000.0, 0.0, 20000.0 };
        case MetaType::pitch:
            return { "st", 0.0, -12.0, 12.0, {}, true };
        case MetaType::seconds:
            return { "s", 0.0, 0.0, 5.0 };
        case MetaType::sustain:
            return { "amp", 1.0, 0.0, 1.0 };
        //integer
        case MetaType::descrete:
            return { "idx", 0.0, 0.0, 9.0 };
        //choice
        case MetaType::waveform:
            return { "", 0.0, 0.0, 9.0, choice::waveform };
        //boolean
        case MetaType::bypass:
            return { "bypass", 0.0, 0.0, 1.0, choice::bypass };
        case MetaType::plusminus:
            return { "plusminus", -1.0, -1.0, 1.0, choice::plusminus, true };
        case MetaType::onoff:
            return { "onoff", 1.0, 0.0, 1.0, choice::onoff };
        case MetaType::momentary:
            return { "momentary", 0.0, 0.0, 1.0, choice::momentary };
        default:
            return { "undfined", 0.0, 0.0, 0.0 };
        }
    }
};

struct WireMeta {
    std::string_view name_;
    std::string_view desc_;
    MetaType type_;
    std::span<const std::string_view> choices;
    bool showPlusMinus{};
    double def_{};
    double min_{};
    double max_{};

    //Use a constexpr constructor to ensure compile-time resolution
    constexpr WireMeta(std::string_view name,
                       std::string_view desc,
                       MetaType type,
                       std::span<const std::string_view> customchoices = {})
      : name_(name)
      , desc_(desc)
      , type_(type)
      , choices(!customchoices.empty() ? customchoices : WireTypeTraits::get(type).choice)
      , showPlusMinus(WireTypeTraits::get(type).showPlusMinus) {}

    //zero-overhead runtime or compile-time query function
    [[nodiscard]] constexpr bool isBipolar() const noexcept {
        return WireTypeTraits::get(type_).min_ < 0.0;
    }

    //zero-overhead runtime or compile-time query function
    [[nodiscard]] constexpr bool isNormalized() const noexcept {
        if (isBipolar()) {
            return WireTypeTraits::get(type_).min_ == -1.0 && WireTypeTraits::get(type_).max_ == 1.0;
        } else {
            return WireTypeTraits::get(type_).min_ == 0.0 && WireTypeTraits::get(type_).max_ == 1.0;
        }
    }
};

template<std::size_t N>
struct ModuleMeta {
    std::string_view name_;
    std::string_view desc_;
    std::array<WireMeta, N> wires_;
};
} //namespace soemdsp::meta
