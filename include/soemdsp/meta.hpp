#pragma once

#include <array>
#include <span>
#include <string_view>

namespace soemdsp::meta {

namespace choice {
static constexpr std::string_view waveform[]  = { "Saw", "Square", "Triangle", "Sine", "Noise" };
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
    phase,
    pitch,
    seconds,
    sustain,

    //integer
    descrete,
    integer_bipolar,

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
    bool displayChoices{};
    bool divideChoicesVisibly{};
    bool wraparound{};
    bool linearSmoothing{ true };
    int maxDigits{ 3 };

    static constexpr const WireTypeTraits get(MetaType t) noexcept {
        switch (t) {
        //decimal
        case MetaType::decimal:
            return { "", 0.0, 0.0, 1.0 };
        case MetaType::decimal_bipolar:
            return { "", 0.0, -1.0, 1.0, {}, true };
        case MetaType::amplitude:
            return { "amp", 1.0, 0.0, 3.0 };
        case MetaType::decibels:
            return { "dB", 0.0, -60.0, 12.0 };
        case MetaType::frequency:
            return { "Hz", 440.0, 0.0, 20000.0, {}, false, false, false, false, true, 5 };
        case MetaType::phase:
            return { "cycle", 0.0, 0.0, 1.0, {}, false, false, false, true };
        case MetaType::pitch:
            return { "st", 0.0, -12.0, 12.0 };
        case MetaType::seconds:
            return { "s", 0.0, 0.0, 5.0 };
        case MetaType::sustain:
            return { "amp", 1.0, 0.0, 1.0 };
        //integer
        case MetaType::descrete:
            return { "idx", 0.0, 0.0, 9.0, {}, false, false, false, false, false };
        case MetaType::integer_bipolar:
            return { "idx", 0.0, -9.0, 9.0, {}, true, false, false, false, false };
        //choice
        case MetaType::waveform:
            return { "", 0.0, 0.0, 4.0, choice::waveform, false, true, true, false, false };
        //boolean
        case MetaType::bypass:
            return { "bypass", 0.0, 0.0, 1.0, choice::bypass, false, true, true, false, false };
        case MetaType::plusminus:
            return { "plusminus", -1.0, -1.0, 1.0, choice::plusminus, true, true, true, false, false };
        case MetaType::onoff:
            return { "onoff", 1.0, 0.0, 1.0, choice::onoff, false, true, true, false, false };
        case MetaType::momentary:
            return { "momentary", 0.0, 0.0, 1.0, choice::momentary, false, true, true, false, false };
        default:
            return { "undefined", 0.0, 0.0, 0.0 };
        }
    }
};

struct WireMeta {
    std::string_view name_;
    std::string_view desc_;
    MetaType type_;
    std::string_view unit_;
    std::span<const std::string_view> choices;
    bool showPlusMinus{};
    bool displayChoices{};
    bool divideChoicesVisibly{};
    bool wraparound{};
    bool linearSmoothing{ true };
    int maxDigits{ 3 };
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
      , unit_(WireTypeTraits::get(type).unit_)
      , choices(!customchoices.empty() ? customchoices : WireTypeTraits::get(type).choice)
      , showPlusMinus(WireTypeTraits::get(type).showPlusMinus)
      , displayChoices(WireTypeTraits::get(type).displayChoices)
      , divideChoicesVisibly(!customchoices.empty() ? true : WireTypeTraits::get(type).divideChoicesVisibly)
      , wraparound(WireTypeTraits::get(type).wraparound)
      , linearSmoothing(WireTypeTraits::get(type).linearSmoothing)
      , maxDigits(WireTypeTraits::get(type).maxDigits)
      , def_(!customchoices.empty() ? 0.0 : WireTypeTraits::get(type).def_)
      , min_(!customchoices.empty() ? 0.0 : WireTypeTraits::get(type).min_)
      , max_(!customchoices.empty()
              ? static_cast<double>(customchoices.size() - 1)
              : WireTypeTraits::get(type).max_) {}

    //zero-overhead runtime or compile-time query function
    [[nodiscard]] constexpr bool isBipolar() const noexcept {
        return min_ < 0.0;
    }

    //zero-overhead runtime or compile-time query function
    [[nodiscard]] constexpr bool isNormalized() const noexcept {
        if (isBipolar()) {
            return min_ == -1.0 && max_ == 1.0;
        } else {
            return min_ == 0.0 && max_ == 1.0;
        }
    }
};

template<std::size_t N>
struct ModuleMeta {
    std::string_view name_;
    std::string_view desc_;
    std::array<WireMeta, N> wires_;
};

static_assert(WireMeta{ "frequency", "", MetaType::frequency }.unit_ == "Hz");
static_assert(WireMeta{ "frequency", "", MetaType::frequency }.def_ == 440.0);
static_assert(WireMeta{ "frequency", "", MetaType::frequency }.min_ == 0.0);
static_assert(WireMeta{ "frequency", "", MetaType::frequency }.max_ == 20000.0);
static_assert(WireMeta{ "frequency", "", MetaType::frequency }.maxDigits == 5);
static_assert(WireMeta{ "amplitude", "", MetaType::amplitude }.maxDigits == 3);
static_assert(WireMeta{ "waveform", "", MetaType::waveform }.choices.size() == 5);
static_assert(WireMeta{ "waveform", "", MetaType::waveform }.max_ == 4.0);
static_assert(WireMeta{ "custom", "", MetaType::waveform, choice::onoff }.choices.size() == 2);
static_assert(WireMeta{ "custom", "", MetaType::waveform, choice::onoff }.def_ == 0.0);
static_assert(WireMeta{ "custom", "", MetaType::waveform, choice::onoff }.max_ == 1.0);
} //namespace soemdsp::meta
