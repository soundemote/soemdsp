#pragma once

#include "soemdsp/sehelper.hpp"
#include "soemdsp/semidi.hpp"

#include <algorithm> // std::equal
#include <cctype>    // std::tolower
#include <map>
#include <string_view>
#include <unordered_map>

namespace soemdsp::plugin {
using std::unordered_map;
using choices_t = unordered_map<int, std::string>;
using std::array;
using std::optional;
using std::string;
using std::string_view;
using std::vector;

inline string_view booleanToString(double v, string_view stringIfFalse = "off", string_view stringIfTrue = "on") {
    if (v) {
        return stringIfTrue;
    }
    return stringIfFalse;
}

// outputs value closest to compareValue found in numberList
inline double nearest(double compareValue, const vector<double>& numberList, optional_ref_t foundIndexOut = {}) {
    if (numberList.empty()) {
        SE_BREAK;
        return {};
    }

    size_t i = 0;

    double minDistance  = std::abs(compareValue - numberList[i]);
    double nearestValue = numberList[i];

    for (; i < numberList.size(); ++i) {
        double distance = std::abs(compareValue - numberList[i]);
        if (distance < minDistance) {
            minDistance  = distance;
            nearestValue = numberList[i];
        }
    }

    if (foundIndexOut)
        foundIndexOut = i;

    return nearestValue;
}

template<typename t>
int compare(t a, t b) {
    return toInt((a > b)) - toInt((a < b));
}

inline double stringToMidiNoteValue(string_view v, int octaveOffset = 0) {
    auto c0 = std::toupper(v[0]);
    if (c0 >= 'A' && c0 <= 'G') {
        auto n0    = c0 - 'A';
        auto sharp = v[1] == '#';
        auto flat  = v[1] == 'b';
        auto oct   = std::atoi(v.data() + 1 + (sharp ? 1 : 0) + (flat ? 1 : 0));

        array<int, 7> noteToPosition{ 9, 11, 0, 2, 4, 5, 7 };
        auto res = noteToPosition[n0] + toInt(sharp) - toInt(flat) + (oct + 1 + octaveOffset) * 12;
        return toDouble(res);
    }

    return toDouble(std::atoi(v.data()));
}

inline string midiNoteValueToString(double v, int octaveOffset = 0) {
    size_t i   = roundToSizeT(v);
    size_t idx = i % 12;
    int octave = toInt(i) / 12;
    return fmt::format("{}{}", midi::noteNames[idx] + midi::noteSharps[idx], octave + octaveOffset);
}

inline bool caseInsensitiveCharCompare(char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

// checks if string is any sort of number and captures the integer value
inline bool isNumber(string_view v, int* valueOut) {
    try {
        *valueOut = std::stoi(v.data());
        return true;
    } catch (std::invalid_argument&) {
        return false;
    }
}

// checks if string is any sort of number and capture the floating point value
inline bool isDecimal(string_view v, double* valueOut) {
    try {
        *valueOut = std::stod(v.data());
        return true;
    } catch (std::invalid_argument&) {
        return false;
    }
}

template<typename key_t, typename value_t>
int findKey(const unordered_map<key_t, value_t>& map, string_view v) {
    for (const auto& pair : map) {
        if (pair.second == v) {
            return pair.first; // Return the corresponding key
        }
    }
    SE_BREAK;
    return 0;
}

inline optional<int> partialMatchString(string_view matchString, const vector<string>& stringList) {
    int i = 0;
    for (auto iter = stringList.begin(); iter != stringList.end(); ++iter) {
        if (std::ranges::equal(iter->substr(0, matchString.length()), matchString, caseInsensitiveCharCompare)) {
            return i;
        }
        ++i;
    }

    return std::nullopt;
}

inline optional<int> partialMatchString(string_view matchString, const choices_t& stringMap) {
    int i = 0;
    for (auto iter = stringMap.begin(); iter != stringMap.end(); ++iter) {
        if (std::ranges::equal(iter->second.substr(0, matchString.length()), matchString, caseInsensitiveCharCompare)) {
            return i;
        }
        ++i;
    }

    return std::nullopt;
}

inline string choiceValueToString(const choices_t& map, int v) {
    auto iter = map.find(v);
    if (iter != map.end()) {
        return iter->second;
    } else {
        SE_BREAK;
        return "";
    }
}

inline int choiceStringToValue(const choices_t& map, string_view v) {
    int res;
    if (isNumber(v, &res)) {
        int i = stringToInt(v);
        if (map.find(i) != map.end()) {
            return i;
        }
    } else {
        if (auto opt = partialMatchString(v, map)) {
            return opt.value();
        }
    }
    SE_BREAK;
    return {};
}

inline double timeStringToValue(string_view v) {
    // TODO: convert stirngs like "300ms" and "5 s"
    double res;
    if (isDecimal(v, &res)) {
        return res;
    } else {
        SE_BREAK;
        return {};
    }
}
} // namespace soemdsp::plugin
