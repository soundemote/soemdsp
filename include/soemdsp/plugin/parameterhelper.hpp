#pragma once

#include <algorithm> // std::equal
#include <cctype>    // std::tolower
#include <map>
#include <string_view>
#include <unordered_map>
#include <string_view>

#include <fmt/format.h>

#include <soemdsp/sehelper.hpp>
#include <soemdsp/semidi.hpp>

namespace soemdsp::plugin::parameterprototype::parameterhelper {
using std::unordered_map;
using optional_ref_t = std::optional<std::reference_wrapper<size_t>>;
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

inline double nearest(double compareValue, const vector<double>& valueList, optional_ref_t foundIndexOut = {}) {
    if (valueList.empty()) {
        debug::STOP();
        return {};
    }

    size_t i = 0;

    double minDistance  = std::abs(compareValue - valueList[i]);
    double nearestValue = valueList[i];

    for (; i < valueList.size(); ++i) {
        double distance = std::abs(compareValue - valueList[i]);
        if (distance < minDistance) {
            minDistance  = distance;
            nearestValue = valueList[i];
        }
    }

    if (foundIndexOut) {
        foundIndexOut = i;
    }

    return nearestValue;
}

template<typename t>
inline int compare(t a, t b) {
    return static_cast<int>((a > b)) - static_cast<int>((a < b));
}

inline double stringToMidiNoteValue(string_view v, int octaveOffset = 0) {
    auto c0 = std::toupper(v[0]);
    if (c0 >= 'A' && c0 <= 'G') {
        auto n0    = c0 - 'A';
        auto sharp = v[1] == '#';
        auto flat  = v[1] == 'b';
        auto oct   = std::atoi(v.data() + 1 + (sharp ? 1 : 0) + (flat ? 1 : 0));

        array<int, 7> noteToPosition{ 9, 11, 0, 2, 4, 5, 7 };
        auto res = noteToPosition[n0] + static_cast<int>(sharp) - static_cast<int>(flat) + (oct + 1 + octaveOffset) * 12;
        return static_cast<double>(res);
    }

    return static_cast<double>(std::atoi(v.data()));
}

inline string midiNoteValueToString(double v, int octaveOffset = 0) {
    int octave = static_cast<int>(v) / 12;
    size_t idx = static_cast<size_t>(v) % 12;
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
inline int findKey(const unordered_map<key_t, value_t>& map, string_view v) {
    for (const auto& pair : map) {
        if (pair.second == v) {
            return pair.first; // Return the corresponding key
        }
    }
    debug::STOP();
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

inline optional<int> partialMatchString(string_view matchString, const unordered_map<int, std::string>& stringMap) {
    int i = 0;
    for (auto iter = stringMap.begin(); iter != stringMap.end(); ++iter) {
        if (std::ranges::equal(iter->second.substr(0, matchString.length()), matchString, caseInsensitiveCharCompare)) {
            return i;
        }
        ++i;
    }

    return std::nullopt;
}

inline string choiceValueToString(const unordered_map<int, std::string>& map, int v) {
    auto iter = map.find(v);
    if (iter != map.end()) {
        return iter->second;
    } else {
        debug::STOP();
        return "";
    }
}
// Function to find the closest iterators for the lower and higher key-value pairs, forgot what i made this for
inline std::pair<std::map<double, double>::const_iterator, std::map<double, double>::const_iterator> findClosestKeys(const std::map<double, double>& myMap, double key) {
    // returns equal to or greater than key
    auto iter = myMap.upper_bound(key);

    if (iter == myMap.begin()) {
        return { iter, std::next(iter) };
    }

    return { std::prev(iter), iter };
}
inline int choiceStringToValue(const unordered_map<int, std::string>& map, std::string_view s) {
    int res;
    if (isNumber(s, &res)) {
        int i = convert::string_to_int(s);
        if (map.find(i) != map.end()) {
            return i;
        }
    } else {
        if (auto opt = partialMatchString(s, map)) {
            return opt.value();
        }
    }
    debug::STOP();
    return {};
}

inline double timeStringToValue(std::string_view s) {
    // TODO: convert strings like "300ms" and "5s"
    double res;
    if (isDecimal(s, &res)) {
        return res;
    }
    else {
        debug::STOP();
        return {};
    }
}
} // namespace soemdsp::plugin
