#pragma once

#include "ParamHelpers.hpp"
#include "soemdsp/operations_string.hpp"
#include "soemdsp/operations_vector.hpp"
#include "soemdsp/curve_functions.hpp"
#include "soemdsp/utility/BeatDivision.hpp"

#include <map>
#include <memory>

namespace soemdsp::plugin {
using std::array;
using std::string;
using std::string_view;
using std::to_string;
using std::vector;

struct ParameterTypeBase {

    ParameterTypeBase() = default;
    virtual ~ParameterTypeBase() = default;

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

    double valueLinear{};    // incoming non-smoothed non-skewed value that may need conversion for the dsp object
    double valueDspDouble{}; // outgoing dsp converted mapped and possibly smoothed value
    int valueDspInteger{};   // outgoing dsp converted mapped and possibly smoothed value
    string valueDspString{};

    std::function<void()> callback        = []() {};
    std::function<void()> delayedCallback = []() {};

    uint32_t flags{}; // optional flags to pass to a subsequent interpreter. Used by CLAP

    // convert a value from skewed curve to linear, usually implemented as curve::Exponential::getInv()
    virtual double mappedToLinear(double v) const {
        return v;
    }
    // convert a value from linear to skewed curve, usually implemented as curve::Exponential::get()
    virtual double linearToMapped(double v) const {
        return v;
    }

    // convert a value string to a number, usually implemented as mappedToLinear(stringToDouble(v));
    virtual double stringToValue(string_view v) const {
        return mappedToLinear(stringToDouble(v));
    }
    // convert a number to a value string, usually implemented as to_string(linearToMapped(v));
    virtual string valueToString(double v) const {
        return to_string(linearToMapped(v));
    }

    // convert a user input value string to a number, usually implemented as linearToMapped(stringToDouble(v));
    virtual double userStringToValue(string_view v) const {
        return linearToMapped(stringToDouble(v));
    }

    // convert xml patch value value used by a dsp object
    // example usage: converting decibels to amplitude if def/min/max values are in decibels
    virtual double patchToDsp(double v) const {
        return v;
    }

    // convert number string to user-friendly display string with units, usually implemented with valueToString() + "unit";
    virtual string displayString(double v) const {
        return valueToString(v);
    };

    // store a mapped value for the dsp object
    void updateValueFromLinear(double v) {
        valueLinear     = v;
        valueDspDouble  = patchToDsp(linearToMapped(v));
        valueDspInteger = toInt(valueDspDouble);
        callback();
    }

    // get value for storing as a string in the xml patch
    virtual string getPatchValueString() {
        switch (type) {
        case Type::CONTINUOUS:
            return str::limitDecimals(to_string(linearToMapped(valueLinear)), 99, 1, 9, true);
            break;
        case Type::DISCRETE:
            return to_string(toInt(linearToMapped(valueLinear)));
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        return {};
    }

    // store a mapped value for the dsp object from the xml patch
    virtual void updateValueFromXml(const string& s) {
        double v        = stringToDouble(s);
        valueLinear     = mappedToLinear(v);
        valueDspDouble  = patchToDsp(v);
        valueDspInteger = toInt(valueDspDouble);
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

struct ParameterTypeBoolean : public ParameterTypeBase {
    ParameterTypeBoolean(int defaultState = 0, string_view stringForFalse = "false", string_view stringForTrue = "true") {
        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = toDouble(defaultState);

        falseTrueStrings[0] = stringForFalse;
        falseTrueStrings[1] = stringForTrue;

        // add to falseTrueStrings true string to stringsToMatchAsTrue list if it's not already there
        if (auto iter = std::ranges::find(stringsToMatchAsTrue, falseTrueStrings[1]); iter == std::ranges::end(stringsToMatchAsTrue)) {
            stringsToMatchAsTrue.insert(stringsToMatchAsTrue.begin(), falseTrueStrings[1]);
        }
    }

    double stringToValue(string_view v) const override {
        if (partialMatchString(v, stringsToMatchAsTrue)) {
            return maxVal;
        }
        return minVal;
    };

    string valueToString(double v) const override {
        return toBool(v) ? falseTrueStrings[1] : falseTrueStrings[0];
    };

    array<string, 2> falseTrueStrings;
    vector<string> stringsToMatchAsTrue{ "true", "1", "on", "yes", "up", "high", "top" };
};

struct ParameterTypeBypass : public ParameterTypeBase {
    ParameterTypeBypass() {
        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = 0;

        flags |= 1 << 4; // CLAP_PARAM_IS_BYPASS;
    }

    double stringToValue(string_view v) const override {
        if (partialMatchString(v, stringsToMatchAsTrue)) {
            return maxVal;
        }
        return minVal;
    };

    string valueToString(double v) const override {
        return toBool(v) ? falseString : stringsToMatchAsTrue[0];
    };

    string falseString{ "online" };
    vector<string> stringsToMatchAsTrue{ "offline", "bypassed" };
};

struct ParameterTypeInteger : public ParameterTypeBase {
    ParameterTypeInteger(int def, int min, int max) {
        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = toDouble(def);
        minVal = toDouble(min);
        maxVal = toDouble(max);
    }

    string displayString(double v) const override {
        return fmt::format("{}", toInt(v));
    }
};

struct ParameterTypeDecimal : public ParameterTypeBase {
    ParameterTypeDecimal(double def = 0.0, double min = 0.0, double max = 1.0, double skew = -0.99) {
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

struct ParameterTypePercent : public ParameterTypeBase {
    ParameterTypePercent(double def = 0.0, double min = 0.0, double max = 1.0) {
        defVal           = def;
        minVal           = min;
        maxVal           = max;
        maxDigits        = 4;
        minDecimalPlaces = 3;
        maxDecimalPlaces = 3;
    }

    double stringToValue(string_view v) const override {
        string s{ v };
        str::removeNonDigits(s);
        return mapNtoN(stringToDouble(s), 0.0, 100.0, minVal, maxVal);
    }
    string valueToString(double v) const override {
        return to_string(mapNtoN(v, minVal, maxVal, 0.0, 100.0));
    }

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxDigits, minDecimalPlaces, maxDecimalPlaces) + "%";
    }
};

struct ParameterTypeChoice : public ParameterTypeBase {
    ParameterTypeChoice(int defaultChoice, const choices_t& customChoices) {
        SE_ERROR(!customChoices.empty(), "no strings found in choice list, check ParameterTypeChoice constructor");

        type    = ParameterTypeBase::Type::DISCRETE;
        choices = customChoices;

        defVal = toDouble(defaultChoice);
        minVal = 0.0;
        for (auto& [val, key] : customChoices) { // find lowest choice value
            minVal = std::min<double>(toDouble(val), minVal);
        }
        maxVal = toDouble(customChoices.size() - 1);
    }

    ParameterTypeChoice(int defaultChoice, std::initializer_list<string> customChoices) {
        SE_ERROR(customChoices.size() > 0, "No strings found in choice list, check ParameterTypeChoice constructor.");

        type  = ParameterTypeBase::Type::DISCRETE;
        int i = 0;
        for (auto& choice : customChoices) {
            choices[i++] = choice;
        }

        defVal = toDouble(defaultChoice);
        minVal = 0.0;
        maxVal = toDouble(customChoices.size() - 1);
    }

    double stringToValue(string_view v) const override {
        return choiceStringToValue(choices, v);
    }
    string valueToString(double v) const override {
        return (choiceValueToString(choices, roundToInt(v)));
    }

    string displayString(double v) const override {
        int c = roundToInt(v);
        if (choices.find(c) != choices.end()) {
            return choices.at(c);
        }
        return {};
    };

    choices_t choices;
};

/* SPECIALIZED */

struct ParameterTypeMidiChannel : public ParameterTypeBase {
    ParameterTypeMidiChannel() {
        type   = ParameterTypeBase::Type::DISCRETE;
        minVal = 0.0;
        maxVal = 15.0;
    }

    double stringToValue(string_view v) const override {
        return toDouble(stringToInt(v) - 1);
    }

    // convert a number to a value string, usually implemented as to_string(linearToMapped(v));
    virtual string valueToString(double v) const {
        return to_string(toInt(linearToMapped(v) + 1.0));
    }
    // string displayString(double v) const override { return valueToString(v); }
};

struct ParameterTypeMidiNote : public ParameterTypeBase {
    ParameterTypeMidiNote() {
        type   = ParameterTypeBase::Type::DISCRETE;
        maxVal = 127.0;
    }

    double stringToValue(string_view v) const override {
        return stringToMidiNoteValue(v);
    }

    string valueToString(double v) const override {
        return midiNoteValueToString(v);
    }
};

struct ParameterTypeFrequency : public ParameterTypeBase {
    ParameterTypeFrequency(double def = 1.0, double min = 0.01, double max = 10.0, double skew = -0.99) {
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

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxDigits, minDecimalPlaces, maxDecimalPlaces, false, true) + " Hz";
    }

    curve::Exponential exponential;
};

struct ParameterTypeTime : public ParameterTypeBase {
    ParameterTypeTime(double def = 0.01f, double min = .0001f, double max = 10.0, double skew = -0.99) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
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

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + " s";
    }

    int maxNumDecimalPlaces{ 5 };

    curve::Exponential exponential;
};

struct ParameterTypeBipolarTime : public ParameterTypeBase {
    ParameterTypeBipolarTime(double def = 0.0f, double min = -1.0, double max = +1.0, double skew = -0.99) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
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

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + " s";
    }

    int maxNumDecimalPlaces{ 5 };

    curve::Sigmoid<curve::Exponential> exponentialS;
};

struct ParameterTypeMultiplier : public ParameterTypeBase {
    ParameterTypeMultiplier(double def = 1.0f, double min = 0.0, double max = 2.0, double skew = -0.99) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
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

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), maxNumDecimalPlaces, 0, maxNumDecimalPlaces) + "x";
    }

    int maxNumDecimalPlaces{ 4 };

    curve::Exponential exponential;
};

struct ParameterTypeBeatDivision : public ParameterTypeBase {
    ParameterTypeBeatDivision(int def = 5, int min = 0, int max = 11, std::shared_ptr<utility::BeatDivision> beatDivisionPtr = std::make_shared<utility::BeatDivision>()) {
        beatDivision = beatDivisionPtr;
        SE_WITHIN_SIZE(min, beatDivision->noteDivisionStrings_);
        SE_WITHIN_SIZE(max, beatDivision->noteDivisionStrings_);
        SE_WITHIN_SIZE(def, beatDivision->noteDivisionStrings_);

        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = def;
        minVal = min;
        maxVal = max;
    }

    std::shared_ptr<utility::BeatDivision> beatDivision;

    string getPatchValueString() override {
        return beatDivision->noteDivisionStrings_[toSizeT(valueDspInteger)];
    }

    void updateValueFromLinear(double v) {
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = toInt(valueDspDouble);
        callback();
    }

    void updateValueFromXml(const string& s) override {
        double v        = toDouble(getIdxFromString(s));
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = toInt(v);
        callback();
    }

    size_t getIdxFromString(string_view v) const {
        std::string s{ v };
        // replace backslash with forwardslash
        str::replace(s, '\\', '/');

        // split the string at slash
        auto tokens = str::split(s, "/");

        // ensure two tokens
        if (tokens.size() == 1)
            tokens.push_back("1");

        // clean strings
        str::removeNonDigits(tokens[0]);
        str::removeNonDigits(tokens[1]);

        // combine tokens to search for beat division string
        string target = tokens[0] + "/" + tokens[1];
        size_t idx    = VEC::indexOf(target, beatDivision->noteDivisionStrings_);

        return toSizeT(roundToDouble(idx));
    }

    // convert a value string to a number, usually implemented as mappedToLinear(stringToDouble(v));
    double stringToValue(string_view v) const override {
        return toDouble(getIdxFromString(v));
    }

    // convert a number to a vlaue string, usually implemented as to_string(linearToMapped(v));
    string valueToString(double v) const override {
        size_t idx = VEC::nearestIndexOf(v, beatDivision->noteDivisionValues_);
        return beatDivision->noteDivisionStrings_[idx];
    }

    string displayString(double v) const override {
        return valueToString(v);
    }

    int maxNumDecimalPlaces{ 5 };
};

struct ParameterTypeBeatModifier : public ParameterTypeBase {
    ParameterTypeBeatModifier(int def = 0, std::shared_ptr<utility::BeatDivision> beatDivisionPtr = std::make_shared<utility::BeatDivision>()) {
        beatDivision = beatDivisionPtr;
        SE_WITHIN_SIZE(def, beatDivision->noteModifiers_);

        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = def;
        minVal = 0.0;
        maxVal = toDouble(beatDivision->noteModifiers_.size() - 1);
    }

    std::shared_ptr<utility::BeatDivision> beatDivision;

    string getPatchValueString() override {
        return beatDivision->noteModifiers_[toSizeT(valueDspInteger)];
    }

    void updateValueFromLinear(double v) {
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = toInt(valueDspDouble);
        callback();
    }

    void updateValueFromXml(const string& s) override {
        double v        = toDouble(getIdxFromString(s));
        valueLinear     = v;
        valueDspDouble  = v;
        valueDspInteger = toInt(v);
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
        return toDouble(getIdxFromString(v));
    }

    string valueToString(double v) const override {
        return beatDivision->noteModifiers_[toSizeT(v)];
    }

    string displayString(double v) const override {
        return valueToString(v);
    }

    int maxNumDecimalPlaces{ 5 };
};

struct ParameterTypeBeatMultiplier : public ParameterTypeBase {
    ParameterTypeBeatMultiplier(
      double def = 1.0f, double min = 0.0, double max = 2.0, double skew = -0.99, std::shared_ptr<soemdsp::utility::BeatDivision> beatDivisionPtr = std::make_shared<utility::BeatDivision>()) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
        defVal = def;
        minVal = min;
        maxVal = max;
        exponential.setup(skew, min, max);
        beatDivision = beatDivisionPtr;
    }

    std::shared_ptr<utility::BeatDivision> beatDivision;

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), 4, 4, 4, false, true) + "x";
    }

    int maxNumDecimalPlaces{ 4 };

    curve::Exponential exponential;
};

struct ParameterTypeSemitones : public ParameterTypeBase {
    ParameterTypeSemitones(double def = 0.0, double min = -36.0, double max = +36.0) {
        defVal = def;
        minVal = min;
        maxVal = max;
    }

    string displayString(double v) const override {
        return (v > 0.0 ? "+" : "") + string(str::limitDecimals(valueToString(v), 3, 2, 2));
    }
};

// Function to find the closest iterators for the lower and higher key-value pairs
inline std::pair<std::map<double, double>::const_iterator, std::map<double, double>::const_iterator> findClosestKeys(const std::map<double, double>& myMap, double key) {
    // returns equal to or greater than key
    auto iter = myMap.upper_bound(key);

    if (iter == myMap.begin()) {
        return { iter, std::next(iter) };
    }

    return { std::prev(iter), iter };
}

struct ParameterTypeDecibelsToAmplitude : public ParameterTypeBase {
    ParameterTypeDecibelsToAmplitude(double defDecibels = -6.0, double minDecibels = -100, double maxDecibels = 0.0, double skew = 0.7) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
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
        return dbToAmp(v);
    }

    string displayString(double v) const override {
        if (v <= decibelsForSilence) {
            return "-inf dB";
        }
        return str::limitDecimals(valueToString(v), 3, 2, 2) + " dB";
    }

    double decibelsForSilence{ -100.0 };
    curve::Exponential exponential;
};

struct ParameterTypeDecibels : public ParameterTypeBase {
    ParameterTypeDecibels(double defDecibels = -6.0, double minDecibels = -100, double maxDecibels = 0.0, double skew = 0.7) {
        type   = ParameterTypeBase::Type::CONTINUOUS;
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

    string displayString(double v) const override {
        return str::limitDecimals(valueToString(v), 3, 2, 2) + " dB";
    }

    curve::Exponential exponential;
};

struct ParameterTypeSmoothingSamples : public ParameterTypeBase {
    ParameterTypeSmoothingSamples(int def = 0, int min = 0, int max = 16384, double skew = 0.0) {
        type   = ParameterTypeBase::Type::DISCRETE;
        defVal = toDouble(def);
        minVal = toDouble(min);
        maxVal = toDouble(max);
        exponential.setup(skew, min, max);
    }

    double mappedToLinear(double v) const override {
        return exponential.getInv(v);
    }
    double linearToMapped(double v) const override {
        return exponential.get(v);
    }

    double stringToValue(string_view v) const override {
        return stringToInt(v);
    }
    string valueToString(double v) const override {
        return fmt::format("{}", v);
    }

    string displayString(double v) const override {
        if (v <= 0) {
            return fmt::format("{} smps [auto]", toInt(SampleRate::block_));
        }
        return fmt::format("{} smps", toInt(v));
    }

    curve::Exponential exponential;
};

// struct ParameterTypeDelayLfoAmp : public ParameterTypeBase {
//     ParameterTypeDelayLfoAmp(double def = 0.0) {
//         type   = ParameterTypeBase::Type::CONTINUOUS;
//         defVal = def;
//         minVal = 0.0;
//         maxVal = 0.5;
//
//         for (size_t i = 0; i < sz; ++i) {
//             double x         = toDouble(i) / toDouble(sz - 1);
//             double linearVal = map0to1(x, minVal, maxVal);
//             double mappedVal = curve::Exponential::get(linearVal, tension, minVal, maxVal);
//
//             linearToMapped222Values[linearVal] = mappedVal;
//             mappedToLinear222Values[mappedVal] = linearVal;
//         }
//     }
//
//     bool shouldSmooth() const override { return true; }
//
//     double mappedToLinear(double v) const override {
//         v                        = std::clamp(v, minVal, maxVal);
//         auto [lowIter, highIter] = findClosestKeys(mappedToLinear222Values, v);
//
//         if (highIter == mappedToLinear222Values.end())
//             return lowIter->second;
//
//         return lerp(v, lowIter->first, highIter->first, lowIter->second, highIter->second);
//     }
//
//     double linearToMapped(double v) const override {
//         v                        = std::clamp(v, minVal, maxVal);
//         auto [lowIter, highIter] = findClosestKeys(linearToMapped222Values, v);
//
//         if (highIter == linearToMapped222Values.end())
//             return lowIter->second;
//
//         return lerp(v, lowIter->first, highIter->first, lowIter->second, highIter->second);
//     }
//
//     double stringToValue(string_view v) const override { return stringToDouble(v); } // convert to percentage
//
//     string valueToString(double v) const override {
//         return std::to_string(v); // convert to percentage
//     };
//
//     string displayString(double v) const override { return string(valueToString(v)) + "%"; }
//
//     const double tension{ -.9 };
//     static constexpr size_t sz{ 32768 };
//     std::map<double, double> mappedToLinear222Values;
//     std::map<double, double> linearToMapped222Values;
// };

} // namespace soemdsp::plugin
