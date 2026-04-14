#pragma once

#include "operations_string.hpp"

using std::string;
using std::string_view;
using std::vector;
using std::max;
using std::min;
//using std::move; // naming conflict

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

    s.reserve(toSizeT(length));
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

string limitDecimals(const string& v, int maxDigits, int minDecimalPlaces, int maxDecimalPlaces, bool doRemoveTrailingZeros, bool allowExtraDecimalForLeadingZero) {
    StringIterator si(v);
    si.movePast(BLANKS);
    si.start();

    string sign;
    string preDotNumber;
    string dot;
    string postDotNumber;

    si.consume(SIGNS, 1);
    sign = si.popMemory();

    si.consume(DIGITS);
    preDotNumber = si.popMemory();
    if (allowExtraDecimalForLeadingZero && preDotNumber == "0") {
        preDotNumber.clear();
    } else if (preDotNumber.empty()) {
        preDotNumber = "0";
    }

    si.consume('.', 1);
    dot = si.popMemory();

    if (dot.empty()) {
        return sign + preDotNumber;
    }

    int digitBudget = max(0, maxDigits - toInt(preDotNumber.size()));
    si.consume(DIGITS, digitBudget + 1);
    postDotNumber     = si.popMemory();
    int decimalBudget = min(digitBudget, maxDecimalPlaces);

    if (decimalBudget < postDotNumber.size()) {
        /* ROUND NUMBER */
        if (postDotNumber[decimalBudget] >= '5') {
            /* ROUND POST DECIMAL NUMBER */
            int pos = max(0, decimalBudget - 1);
            while (pos >= 0 && postDotNumber[pos] == '9') {
                --pos;
            }
            /* JUST ROUND FROM THE FIRST DECIMAL */
            if (pos == 0 && postDotNumber[pos] >= '5' && decimalBudget <= 0) {
                ++preDotNumber[preDotNumber.size() - 1];
                postDotNumber.clear();
            } else if (pos == -1) {
                /* ROUND PRE DECIMAL NUMBER */
                pos = preDotNumber.size() - 1;
                while (pos > 0 && preDotNumber[pos] == '9') {
                    --pos;
                }
                if (pos >= 0) {
                    /* RETURN INTEGER */
                    if (preDotNumber[pos] != '9') {
                        ++preDotNumber[pos];
                        size_t sz = preDotNumber.size();
                        /* ZERO OUT ALL 9s AFTER INCREASING NON-9 DIGIT */
                        for (int i = pos + 1; i < preDotNumber.size(); ++i) {
                            preDotNumber[i] = '0';
                        }
                    } else {
                        int originalPreDotNumberSize = preDotNumber.size();
                        preDotNumber                 = '1';
                        str::padRight(preDotNumber, "0", originalPreDotNumberSize + 1);
                        --digitBudget;
                    }
                    if (!doRemoveTrailingZeros && minDecimalPlaces >= 0) {
                        postDotNumber.clear();
                        padRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
                        if (!postDotNumber.empty()) {
                            return sign + preDotNumber + '.' + postDotNumber;
                        }
                    }
                    return sign + preDotNumber;
                }
            } else {
                /* RETURN DECIMAL */
                postDotNumber.resize(pos + 1);
                ++postDotNumber[pos];
                if (allowExtraDecimalForLeadingZero && preDotNumber == "0" && minDecimalPlaces >= digitBudget) {
                    ++digitBudget;
                    if (!doRemoveTrailingZeros) {
                        padRight(postDotNumber, "0", digitBudget);
                    }
                    return sign + '.' + postDotNumber;
                }
                if (!doRemoveTrailingZeros) {
                    padRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
                }
                return sign + preDotNumber + '.' + postDotNumber;
            }
        } else {
            postDotNumber.resize(decimalBudget);
        }
    }

    if (doRemoveTrailingZeros && !postDotNumber.empty()) {
        removeTrailingZeros(postDotNumber);
    } else {
        padRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
    }

    if (postDotNumber.empty()) {
        return sign + preDotNumber;
    }
    return sign + preDotNumber + '.' + postDotNumber;
}
} // namespace soemdsp::str
