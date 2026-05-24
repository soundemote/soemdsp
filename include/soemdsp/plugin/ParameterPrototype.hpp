#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <soemdsp.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>

#include <soemdsp/musical/BeatDivision.hpp>
#include <soemdsp/utility/curve_functions.hpp>
#include <soemdsp/utility/operations_string.hpp>
#include <soemdsp/utility/operations_vector.hpp>

#include "parameterhelper.hpp"

// for flags see https://github.com/free-audio/clap/blob/main/include/clap/ext/params.h

/* parameter prototype used with Parameter class to setup how values go from value to string to human readable to internal value vs external value, etc.
 * all in one place.
 * there's two main types CONTINUOUS or DISCRETE
 * from there there are many types
 * types from boolean to string based parameters to all sorts of curves to ones specifically for envelope timimg, amplitde, volume, to basic and simple
 */
namespace soemdsp::plugin::parameterprototype {

struct ParameterPrototype {
    ParameterPrototype()          = default;
    virtual ~ParameterPrototype() = default;

    enum class Type {
        CONTINUOUS,
        DISCRETE
    } type{};

    double defVal{ 0.0 };
    double minVal{ 0.0 };
    double maxVal{ 1.0 };

    int maxDigits{ 5 }; // maximum number of digits to display when returning a display string
    int minDecimalPlaces{ 0 };
    int maxDecimalPlaces{ 5 };

    double valueLinear{};       // incoming non-smoothed non-skewed value that may need conversion for the dsp object
    double valueDspDouble{};    // outgoing dsp converted mapped and possibly smoothed value
    int valueDspInteger{};      // outgoing dsp converted mapped and possibly smoothed value, rounded down ultimately (but internally value is as accurate as possible knowing final destination is rounded
                                // down)
    float valueFloatIntenger{}; // float outgoing dsp converted mapped and possibly smoothed value
    std::string valueDspString{};    // string outgoing

    void (*callback)() = op::empty;
    void (*delayedCallback)() = op::empty;

    uint32_t flags{}; // optional flags to pass to a subsequent interpreter. Used by CLAP

    // convert a value from skewed curve to linear, usually implemented as curve::Exponential::getInv()
    virtual double mappedToLinear(double v) const {
        return v;
    }
    // convert a value from linear to skewed curve, usually implemented as curve::Exponential::get()
    virtual double linearToMapped(double v) const {
        return v;
    }

    // convert a value string to a number, usually implemented as mappedToLinear(c::doubletostring(v));
    virtual double stringToValue(string_view s) const {
        return mappedToLinear(convert::string_to_double(s));
    }
    // convert a number to a value string, usually implemented as to_string(linearToMapped(v));
    virtual std::string valueToString(double v) const {
        return convert::double_to_string(linearToMapped(v));
    }

    // convert a user input value string to a number, usually implemented as linearToMapped(c::doubletostring(v));
    virtual double userStringToValue(string_view s) const {
        return linearToMapped(convert::string_to_double(s));
    }

    // convert xml patch value value used by a dsp object
    // example usage: converting decibels to amplitude if def/min/max values are in decibels
    virtual double patchToDsp(double v) const {
        return v;
    }

    // convert number string to user-friendly display string with units, usually implemented with valueToString() + "unit";
    virtual std::string displayString(double v) const {
        return valueToString(v);
    };

    // store a mapped value for the dsp object
    void updateValueFromLinear(double v) {
        valueLinear        = v;
        valueDspDouble     = patchToDsp(linearToMapped(v));
        valueDspInteger    = static_cast<int>(valueDspDouble);
        valueFloatIntenger = static_cast<float>(valueDspDouble);
        callback();
    }

    // get value for storing as a string in the xml patch
    virtual std::string getPatchValueString() {
        switch (type) {
        case Type::CONTINUOUS:
            return str::limitDecimals(convert::double_to_string(linearToMapped(valueLinear)), 99, 1, 9, true);
            break;
        case Type::DISCRETE:
            return convert::double_to_string(linearToMapped(valueLinear));
            break;
        default:
            debug::FAIL();
        }
        return {};
    }

    // store a mapped value for the dsp object from the xml patch
    virtual void updateValueFromXml(const std::string& s) {
        double v        = convert::string_to_double(s);
        valueLinear     = mappedToLinear(v);
        valueDspDouble  = patchToDsp(v);
        valueDspInteger = static_cast<int>(valueDspDouble);
        callback();
    }

    // get the dsp object value as a double
    double getDspDouble() {
        return valueDspDouble;
    }

    // get the dsp object value as int, double is rounded down
    int getDspInt() {
        return valueDspInteger;
    }

    // get the dsp object value as bool if value is more than 0.5
    bool getDspBool() {
        return valueDspInteger != 0;
    }
};

/* BASIC */
struct Bypass : public ParameterPrototype {
    Bypass() {
        type   = ParameterPrototype::Type::DISCRETE;
        defVal = 0;

        flags |= 1 << 4; // CLAP_PARAM_IS_BYPASS;
    }

    double stringToValue(string_view v) const override {
        if (parameterhelper::partialMatchString(v, stringsToMatchAsTrue)) {
            return maxVal;
        }
        return minVal;
    };

    std::string valueToString(double v) const override {
        return convert::double_to_bool(v) ? falseString : stringsToMatchAsTrue[0];
    };

    std::string falseString{ "online" };
    std::vector<std::string> stringsToMatchAsTrue{ "offline", "bypassed" };
};

struct Boolean : public ParameterPrototype {
    Boolean(int defaultState = 0, string_view stringForFalse = "false", string_view stringForTrue = "true") {
        type   = ParameterPrototype::Type::DISCRETE;
        defVal = static_cast<double>(defaultState);

        falseTrueStrings[0] = stringForFalse;
        falseTrueStrings[1] = stringForTrue;

        // add to falseTrueStrings true string to stringsToMatchAsTrue list if it's not already there
        if (auto iter = std::ranges::find(stringsToMatchAsTrue, falseTrueStrings[1]); iter == std::ranges::end(stringsToMatchAsTrue)) {
            stringsToMatchAsTrue.insert(stringsToMatchAsTrue.begin(), falseTrueStrings[1]);
        }
    }

    double stringToValue(string_view v) const override {
        if (parameterhelper::partialMatchString(v, stringsToMatchAsTrue)) {
            return maxVal;
        }
        return minVal;
    };

    std::string valueToString(double v) const override {
        return convert::double_to_bool(v) ? falseTrueStrings[1] : falseTrueStrings[0];
    };

    std::array<std::string, 2> falseTrueStrings;
    std::vector<std::string> stringsToMatchAsTrue{ "true", "1", "on", "yes", "up", "high", "top" };
};

struct Integer : public ParameterPrototype {
    Integer(int def, int min, int max) {
        type   = ParameterPrototype::Type::DISCRETE;
        defVal = static_cast<double>(def);
        minVal = static_cast<double>(min);
        maxVal = static_cast<double>(max);
    }

    std::string displayString(double v) const override {
        return fmt::format("{}", static_cast<int>(v));
    }
};

struct Decimal : public ParameterPrototype {
    Decimal(double def = 0.0, double min = 0.0, double max = 1.0, double skew = -0.99) {
        defVal = def;
        minVal = min;
        maxVal = max;
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    curve::Exponential exponential;
};

/* ADVANCED */
struct Percent : public ParameterPrototype {
    Percent(double def = 0.0, double min = 0.0, double max = 1.0) {
        defVal           = def;
        minVal           = min;
        maxVal           = max;
        maxDigits        = 4;
        minDecimalPlaces = 3;
        maxDecimalPlaces = 3;
    }

    double stringToValue(string_view v) const override {
        std::string s{ v };
        str::removeNonDigits(s);
        return math::mapNtoN(convert::string_to_double(s), 0.0, 100.0, minVal, maxVal);
    }
    std::string valueToString(double v) const override {
        return convert::double_to_string(math::mapNtoN(v, minVal, maxVal, 0.0, 100.0));
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxDigits, minDecimalPlaces, maxDecimalPlaces) + "%";
    }
};

struct Choice : public ParameterPrototype {
    Choice(int defaultChoice, const unordered_map<int, std::string>& customChoices) {
        debug::CHECK(!customChoices.empty(), "no strings found in choice list, check Choice constructor");

        type    = ParameterPrototype::Type::DISCRETE;
        choices = customChoices;

        defVal = static_cast<double>(defaultChoice);
        minVal = 0.0;
        for (auto& [val, key] : customChoices) { // find lowest choice value
            minVal = std::min<double>(static_cast<double>(val), minVal);
        }
        maxVal = static_cast<double>(customChoices.size() - 1);
    }

    Choice(int defaultChoice, std::initializer_list<std::string> customChoices) {
        debug::CHECK(customChoices.size() > 0, "No strings found in choice list, check Choice constructor.");

        type  = ParameterPrototype::Type::DISCRETE;
        int i = 0;
        for (auto& choice : customChoices) {
            choices[i++] = choice;
        }

        defVal = static_cast<double>(defaultChoice);
        minVal = 0.0;
        maxVal = static_cast<double>(customChoices.size() - 1);
    }

    double stringToValue(string_view v) const override {
        return parameterhelper::choiceStringToValue(choices, v);
    }
    std::string valueToString(double v) const override {
        return parameterhelper::choiceValueToString(choices, math::roundint(v));
    }

    std::string displayString(double v) const override {
        int c = math::roundint(v);
        if (choices.find(c) != choices.end()) {
            return choices.at(c);
        }
        return {};
    };

    unordered_map<int, std::string> choices;
};

/* SPECIALIZED */
struct MidiChannel : public ParameterPrototype {
    MidiChannel() {
        type   = ParameterPrototype::Type::DISCRETE;
        minVal = 0.0;
        maxVal = 15.0;
    }

    double stringToValue(string_view v) const override {
        return convert::string_to_double(v) - 1.0;
    }

    // convert a number to a value string, usually implemented as to_string(linearToMapped(v));
    virtual std::string valueToString(double v) const {
        return convert::double_to_string(static_cast<int>(linearToMapped(v) + 1.0));
    }
    // string displayString(double v) const override { return valueToString(v); }
};

struct MidiNote : public ParameterPrototype {
    MidiNote() {
        type   = ParameterPrototype::Type::DISCRETE;
        maxVal = 127.0;
    }

    double stringToValue(string_view v) const override {
        return parameterhelper::stringToMidiNoteValue(v);
    }

    std::string valueToString(double v) const override {
        return parameterhelper::midiNoteValueToString(v);
    }
};

struct Frequency : public ParameterPrototype {
    Frequency(double def = 1.0, double min = 0.01, double max = 10.0, double skew = -0.99) {
        defVal           = def;
        minVal           = min;
        maxVal           = max;
        maxDigits        = 5;
        minDecimalPlaces = 5;
        maxDecimalPlaces = 5;
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }

    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxDigits, minDecimalPlaces, maxDecimalPlaces, false, true) + " Hz";
    }

    curve::Exponential exponential;
};

struct Time : public ParameterPrototype {
    Time(double def = 0.01f, double min = .0001f, double max = 10.0, double skew = -0.99) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = def;
        minVal = min;
        maxVal = max;
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + " s";
    }

    int maxNumDecimalPlaces{ 5 };

    curve::Exponential exponential;
};

struct BipolarTime : public ParameterPrototype {
    BipolarTime(double def = 0.0f, double min = -1.0, double max = +1.0, double skew = -0.99) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = def;
        minVal = min;
        maxVal = max;
        exponentialS.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponentialS.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponentialS.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + " s";
    }

    int maxNumDecimalPlaces{ 5 };

    curve::Sigmoid<curve::Exponential> exponentialS;
};

struct Multiplier : public ParameterPrototype {
    Multiplier(double def = 1.0f, double min = 0.0, double max = 2.0, double skew = -0.99) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = def;
        minVal = min;
        maxVal = max;
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + "x";
    }

    int maxNumDecimalPlaces{ 4 };

    curve::Exponential exponential;
};

struct Beats : public ParameterPrototype {
    Beats(int def = 5, int min = 0, int max = 11, std::shared_ptr<musical::BeatDivision> beatDivisionPtr = std::make_shared<musical::BeatDivision>()) {
        beatDivision_ = beatDivisionPtr;
        debug::WITHINSIZE(min, beatDivision_->noteDivisionStrings_);
        debug::WITHINSIZE(max, beatDivision_->noteDivisionStrings_);
        debug::WITHINSIZE(def, beatDivision_->noteDivisionStrings_);

        type   = ParameterPrototype::Type::DISCRETE;
        defVal = def;
        minVal = min;
        maxVal = max;
    }

    std::shared_ptr<musical::BeatDivision> beatDivision_;

    std::string getPatchValueString() override {
        return beatDivision_->noteDivisionStrings_[static_cast<size_t>(valueDspInteger)];
    }

    void updateValueFromLinear(double v) {
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = static_cast<int>(valueDspDouble);
        callback();
    }

    void updateValueFromXml(const std::string& s) override {
        double v        = static_cast<double>(getIdxFromString(s));
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = static_cast<int>(v);
        callback();
    }

    size_t getIdxFromString(string_view v) const {
        std::string s{ v };
        // replace backslash with forwardslash
        str::replace(s, '\\', '/');

        // split the string at slash
        auto tokens = str::split(s, "/");

        // ensure two tokens
        if (tokens.size() == 1) {
            tokens.push_back("1");
        }

        // clean strings
        str::removeNonDigits(tokens[0]);
        str::removeNonDigits(tokens[1]);

        // combine tokens to search for beat division string
        return memop::indexOf(tokens[0] + "/" + tokens[1], beatDivision_->noteDivisionStrings_);
    }

    // convert a value string to a number, usually implemented as mappedToLinear(c::doubletostring(v));
    double stringToValue(string_view v) const override {
        return static_cast<double>(getIdxFromString(v));
    }

    // convert a number to a value string, usually implemented as to_string(linearToMapped(v));
    std::string valueToString(double v) const override {
        size_t idx = memop::nearestIndexOf(v, beatDivision_->noteDivisionValues_);
        return beatDivision_->noteDivisionStrings_[idx];
    }

    std::string displayString(double v) const override {
        return valueToString(v);
    }

    int maxNumDecimalPlaces{ 5 };
};

struct BeatModifier : public ParameterPrototype {
    BeatModifier(int def = 0, std::shared_ptr<musical::BeatDivision> beatDivisionPtr = std::make_shared<musical::BeatDivision>()) {
        beatDivision_ = beatDivisionPtr;
        debug::WITHINSIZE(def, beatDivision_->noteModifiers_);

        type   = ParameterPrototype::Type::DISCRETE;
        defVal = def;
        minVal = 0.0;
        maxVal = static_cast<double>(beatDivision_->noteModifiers_.size() - 1);
    }

    std::shared_ptr<musical::BeatDivision> beatDivision_;

    std::string getPatchValueString() override {
        return beatDivision_->noteModifiers_[static_cast<size_t>(valueDspInteger)];
    }

    void updateValueFromLinear(double v) {
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = static_cast<int>(valueDspDouble);
        callback();
    }

    void updateValueFromXml(const std::string& s) override {
        double v        = static_cast<double>(getIdxFromString(s));
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = static_cast<int>(v);
        callback();
    }

    size_t getIdxFromString(string_view v) const {
        if (!v.empty()) {
            if (soemdsp::cha::isAny(v[0], "1.dD")) { // 1, ., [Dd]otted
                return 1;
            } else if (soemdsp::cha::isAny(v[0], "2tT")) { // 2, [Tt]riplet
                return 2;
            }
        }
        return 0;
    }

    double stringToValue(string_view v) const override {
        return static_cast<double>(getIdxFromString(v));
    }

    std::string valueToString(double v) const override {
        return beatDivision_->noteModifiers_[static_cast<size_t>(v)];
    }

    std::string displayString(double v) const override {
        return valueToString(v);
    }

    int maxNumDecimalPlaces{ 5 };
};

struct BeatMultiplier : public ParameterPrototype {
    BeatMultiplier(
      double def = 1.0f, double min = 0.0, double max = 2.0, double skew = -0.99, std::shared_ptr<soemdsp::musical::BeatDivision> beatDivisionPtr = std::make_shared<musical::BeatDivision>()) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = def;
        minVal = min;
        maxVal = max;
        exponential.setup(skew, min, max);
        beatDivision = beatDivisionPtr;
    }

    std::shared_ptr<musical::BeatDivision> beatDivision;

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), 4, 4, 4, false, true) + "x";
    }

    int maxNumDecimalPlaces{ 4 };

    curve::Exponential exponential;
};

struct Semitones : public ParameterPrototype {
    Semitones(double def = 0.0, double min = -36.0, double max = +36.0) {
        defVal = def;
        minVal = min;
        maxVal = max;
    }

    std::string displayString(double v) const override {
        return (v > 0.0 ? "+" : "") + std::string(str::limitDecimals(valueToString(v), 3, 2, 2));
    }
};

struct DecibelsToAmplitude : public ParameterPrototype {
    DecibelsToAmplitude(double defDecibels = -6.0, double minDecibels = -100, double maxDecibels = 0.0, double skew = 0.7) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = defDecibels;
        minVal = minDecibels;
        maxVal = maxDecibels;
        exponential.setup(skew, minDecibels, maxDecibels);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    double patchToDsp(double v) const override {
        if (v <= decibelsForSilence) {
            return 0.0;
        }
        return convert::db_to_amp(v);
    }

    std::string displayString(double v) const override {
        if (v <= decibelsForSilence) {
            return "-inf dB";
        }
        return str::limitDecimals(valueToString(v), 3, 2, 2) + " dB";
    }

    double decibelsForSilence{ -100.0 };
    curve::Exponential exponential;
};

struct Decibels : public ParameterPrototype {
    Decibels(double defDecibels = -6.0, double minDecibels = -100, double maxDecibels = 0.0, double skew = 0.7) {
        type   = ParameterPrototype::Type::CONTINUOUS;
        defVal = defDecibels;
        minVal = minDecibels;
        maxVal = maxDecibels;
        exponential.setup(skew, minDecibels, maxDecibels);
    }

    // convert a value from skewed curve to linear, usually implemented with curve::Exponential::getInv() vice versa
    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    // convert a value from linear to skewed curve, usually implemented with curve::Exponential::get() vice versa
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    std::string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), 3, 2, 2) + " dB";
    }

    curve::Exponential exponential;
};

struct SmoothingSamples : public ParameterPrototype {
    SmoothingSamples(int def = 0, int min = 0, int max = 16384, double skew = 0.0) {
        type   = ParameterPrototype::Type::DISCRETE;
        defVal = static_cast<double>(def);
        minVal = static_cast<double>(min);
        maxVal = static_cast<double>(max);
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    double stringToValue(string_view s) const override {
        return convert::string_to_double(s);
    }
    std::string valueToString(double v) const override {
        return fmt::format("{}", v);
    }

    std::string displayString(double v) const override {
        if (v <= 0) {
            return fmt::format("{} smps [auto]", static_cast<int>(SampleRate::block_));
        }
        return fmt::format("{} smps", static_cast<int>(v));
    }

    curve::Exponential exponential;
};

} // namespace soemdsp::plugin::parameterprototype
#pragma endregion
