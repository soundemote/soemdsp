#pragma once

#include "../operations_string.hpp"

#include <fmt/format.h>

using std::max;
using std::min;
using std::string;
using std::string_view;
using std::vector;
// using std::move; // naming conflict

namespace soemdsp::str {

vector<string> split(const string& s, string_view delimiter) {
    vector<string> tokens;
    size_t start = 0, end = 0;

    while ((end = s.find(delimiter, start)) != string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
    }

    string lastToken = s.substr(start);
    if (!lastToken.empty()) {
        tokens.push_back(lastToken); // Add the last token
    }

    return tokens;
}
void removeWhiteSpace(string& s) {
    string ss;
    ss.reserve(s.length());

    for (char c : s) {
        if (!cha::isWhiteSpace(c)) {
            ss += c;
        }
    }

    s = std::move(ss);
}
void removeNonDigits(string& s) {
    string ss;
    ss.reserve(s.length());

    for (char c : s) {
        if (cha::isDigit(c)) {
            ss += c;
        }
    }

    s = std::move(ss);
}

void replace(string& s, char matchChar, char replaceChar) {
    for (char& c : s) {
        if (c == matchChar) {
            c = replaceChar;
        }
    }
}

void padRight(string& s, string_view padding, int length) {
    if (length <= 0 || s.length() >= length) {
        return;
    }

    s.reserve(static_cast<size_t>(length));
    for (size_t i = s.length(); i < length; ++i) {
        s += padding;
    }
}

void removeTrailingZeros(string& s) {
    StringIterator si(s);
    si.toEnd();
    si.backTo(NONZERODIGITS);
    si.move();
    si.remember();
    s = si.popMemory();
}

string time(double seconds, int precision) {
    double threshold = std::pow(10, -precision);
    if (seconds < threshold) {
        return fmt::format("{:.1f}ms", seconds * 1000.0, precision);
    } else {
        return fmt::format("{:.2f}s", seconds, precision);
    }
}

namespace {
string incrementWholeDigits(string whole) {
    if (whole.empty()) {
        return "1";
    }
    for (int i = static_cast<int>(whole.size()) - 1; i >= 0; --i) {
        if (whole[static_cast<size_t>(i)] != '9') {
            ++whole[static_cast<size_t>(i)];
            return whole;
        }
        whole[static_cast<size_t>(i)] = '0';
    }
    return "1" + whole;
}

void trimTrailingZeros2(string& value) {
    while (!value.empty() && value.back() == '0') {
        value.pop_back();
    }
}

void roundLimitedDecimals(string& whole, string& decimals, int decimalPlaces) {
    if (decimalPlaces <= 0) {
        whole = incrementWholeDigits(whole);
        decimals.clear();
        return;
    }

    decimals.resize(static_cast<size_t>(decimalPlaces), '0');
    for (int i = decimalPlaces - 1; i >= 0; --i) {
        if (decimals[static_cast<size_t>(i)] != '9') {
            ++decimals[static_cast<size_t>(i)];
            return;
        }
        decimals[static_cast<size_t>(i)] = '0';
    }
    whole = incrementWholeDigits(whole);
}
} // namespace

string limitDecimals(const string& v, int maxDigits, int minDecimalPlaces, int maxDecimalPlaces, bool removeTrailingZeros, bool allowExtraDecimalForLeadingZero) {
    size_t index = 0;
    while (index < v.size() && cha::isWhiteSpace(v[index])) {
        ++index;
    }

    string sign;
    if (index < v.size() && (v[index] == '+' || v[index] == '-')) {
        sign += v[index];
        ++index;
    }

    const size_t wholeStart = index;
    while (index < v.size() && cha::isDigit(v[index])) {
        ++index;
    }

    string whole = v.substr(wholeStart, index - wholeStart);
    const bool omitLeadingZero = allowExtraDecimalForLeadingZero && whole == "0";
    if (omitLeadingZero) {
        whole.clear();
    } else if (whole.empty()) {
        whole = "0";
    }

    if (index >= v.size() || v[index] != '.') {
        return sign + whole;
    }
    ++index;

    const int boundedMaxDigits       = max(0, maxDigits);
    const int boundedMinDecimals     = max(0, minDecimalPlaces);
    const int boundedMaxDecimals     = max(0, maxDecimalPlaces);
    int digitBudget                  = max(0, boundedMaxDigits - static_cast<int>(whole.size()));
    int decimalPlaces                = min(digitBudget, boundedMaxDecimals);
    string decimals;
    decimals.reserve(static_cast<size_t>(decimalPlaces));
    for (int i = 0; i < decimalPlaces && index < v.size() && cha::isDigit(v[index]); ++i, ++index) {
        decimals += v[index];
    }
    const bool shouldRound = index < v.size() && cha::isDigit(v[index]) && v[index] >= '5';

    if (shouldRound) {
        roundLimitedDecimals(whole, decimals, decimalPlaces);
        digitBudget   = max(0, boundedMaxDigits - static_cast<int>(whole.size()));
        decimalPlaces = min({ static_cast<int>(decimals.size()), digitBudget, boundedMaxDecimals });
        decimals.resize(static_cast<size_t>(decimalPlaces));
    }

    if (removeTrailingZeros) {
        trimTrailingZeros2(decimals);
    } else {
        padRight(decimals, "0", min(digitBudget, boundedMinDecimals));
    }

    if (decimals.empty()) {
        return sign + whole;
    }
    const bool omitWhole = omitLeadingZero && whole.empty();
    return sign + (omitWhole ? string{} : whole) + "." + decimals;
}
} // namespace soemdsp::str
