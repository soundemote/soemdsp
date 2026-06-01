#include <soemdsp/utility/operations_string.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {
using std::max;
using std::min;
using std::string;
using std::string_view;

void oldPadRight(string& s, string_view padding, int length) {
    if (length <= 0 || s.length() >= static_cast<size_t>(length)) {
        return;
    }
    s.reserve(static_cast<size_t>(length));
    for (size_t i = s.length(); i < static_cast<size_t>(length); ++i) {
        s += padding;
    }
}

void oldRemoveTrailingZeros(string& s) {
    while (!s.empty() && s.back() == '0') {
        s.pop_back();
    }
}

string limitDecimalsOriginal(const string& v, int maxDigits, int minDecimalPlaces, int maxDecimalPlaces, bool doRemoveTrailingZeros, bool allowExtraDecimalForLeadingZero) {
    soemdsp::str::StringIterator si(v);
    si.movePast(soemdsp::str::BLANKS);
    si.start();

    string sign;
    string preDotNumber;
    string dot;
    string postDotNumber;

    si.consume(soemdsp::str::SIGNS, 1);
    sign = si.popMemory();

    si.consume(soemdsp::str::DIGITS);
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

    int digitBudget = max(0, maxDigits - static_cast<int>(preDotNumber.size()));
    si.consume(soemdsp::str::DIGITS, digitBudget + 1);
    postDotNumber     = si.popMemory();
    int decimalBudget = min(digitBudget, maxDecimalPlaces);

    if (decimalBudget < static_cast<int>(postDotNumber.size())) {
        if (postDotNumber[static_cast<size_t>(decimalBudget)] >= '5') {
            int pos = max(0, decimalBudget - 1);
            while (pos >= 0 && postDotNumber[static_cast<size_t>(pos)] == '9') {
                --pos;
            }
            if (pos == 0 && postDotNumber[static_cast<size_t>(pos)] >= '5' && decimalBudget <= 0) {
                ++preDotNumber[preDotNumber.size() - 1];
                postDotNumber.clear();
            } else if (pos == -1) {
                pos = static_cast<int>(preDotNumber.size()) - 1;
                while (pos > 0 && preDotNumber[static_cast<size_t>(pos)] == '9') {
                    --pos;
                }
                if (pos >= 0) {
                    if (preDotNumber[static_cast<size_t>(pos)] != '9') {
                        ++preDotNumber[static_cast<size_t>(pos)];
                        for (int i = pos + 1; i < static_cast<int>(preDotNumber.size()); ++i) {
                            preDotNumber[static_cast<size_t>(i)] = '0';
                        }
                    } else {
                        int originalPreDotNumberSize = static_cast<int>(preDotNumber.size());
                        preDotNumber                 = '1';
                        oldPadRight(preDotNumber, "0", originalPreDotNumberSize + 1);
                        --digitBudget;
                    }
                    if (!doRemoveTrailingZeros && minDecimalPlaces >= 0) {
                        postDotNumber.clear();
                        oldPadRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
                        if (!postDotNumber.empty()) {
                            return sign + preDotNumber + '.' + postDotNumber;
                        }
                    }
                    return sign + preDotNumber;
                }
            } else {
                postDotNumber.resize(static_cast<size_t>(pos + 1));
                ++postDotNumber[static_cast<size_t>(pos)];
                if (allowExtraDecimalForLeadingZero && preDotNumber == "0" && minDecimalPlaces >= digitBudget) {
                    ++digitBudget;
                    if (!doRemoveTrailingZeros) {
                        oldPadRight(postDotNumber, "0", digitBudget);
                    }
                    return sign + '.' + postDotNumber;
                }
                if (!doRemoveTrailingZeros) {
                    oldPadRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
                }
                return sign + preDotNumber + '.' + postDotNumber;
            }
        } else {
            postDotNumber.resize(static_cast<size_t>(decimalBudget));
        }
    }

    if (doRemoveTrailingZeros && !postDotNumber.empty()) {
        oldRemoveTrailingZeros(postDotNumber);
    } else {
        oldPadRight(postDotNumber, "0", min(digitBudget, minDecimalPlaces));
    }

    if (postDotNumber.empty()) {
        return sign + preDotNumber;
    }
    return sign + preDotNumber + '.' + postDotNumber;
}
} // namespace

struct LimitDecimalsCase {
    std::string input;
    int maxDigits{};
    int minDecimalPlaces{};
    int maxDecimalPlaces{};
    bool removeTrailingZeros{};
    bool allowExtraDecimalForLeadingZero{};
    std::string expected;
};

int main() {
    const std::vector<LimitDecimalsCase> cases{
        { "1.999995", 3, 2, 5, false, false, "2.00" },
        { "9.999995", 3, 2, 5, false, false, "10.0" },
        { "9999.99995", 3, 2, 5, false, false, "10000" },
        { "9.999995", 3, 4, 5, true, false, "10" },
        { "9.999995", 4, 4, 5, false, false, "10.00" },
        { "0.999995", 4, 4, 5, false, false, "1.000" },
        { " +.987655", 3, 2, 5, true, false, "+0.99" },
        { " + .987655", 3, 2, 5, true, false, "+0" },
        { " 46 .987655", 3, 2, 5, true, false, "46" },
        { " abc .987655", 3, 2, 5, true, false, "0" },
        { "abc.987655", 3, 2, 5, true, false, "0" },
        { "1.987655", 3, 2, 5, true, false, "1.99" },
        { "0.987655", 3, 3, 3, false, true, ".988" },
        { "21.987655", 3, 3, 3, false, true, "22.0" },
        { "321.987655", 3, 3, 3, false, true, "322" },
        { "321.1234", 3, 3, 3, false, true, "321" },
        { "21.987655", 3, 2, 5, false, false, "22.0" },
        { "21.987655", 3, 2, 5, true, false, "22" },
        { "321.987655", 3, 2, 5, true, false, "322" },
        { "1.4444444", 3, 2, 5, false, false, "1.44" },
        { "9.4444444", 3, 2, 5, false, false, "9.44" },
        { "9999.4444444", 3, 2, 5, false, false, "9999" },
        { "9.4444444", 3, 4, 5, true, false, "9.44" },
        { "9.4444444", 4, 4, 5, false, false, "9.444" },
        { "0.4444444", 4, 4, 5, false, false, "0.444" },
        { " +.4444444", 3, 2, 5, true, false, "+0.44" },
        { "1.4444444", 3, 2, 5, true, false, "1.44" },
        { "21.4444444", 3, 2, 5, false, false, "21.4" },
        { "21.4444444", 3, 2, 5, true, false, "21.4" },
        { "321.4444444", 3, 2, 5, true, false, "321" },
    };

    int currentFailures = 0;
    int oldCurrentMismatches = 0;
    for (const auto& test : cases) {
        const std::string result = soemdsp::str::limitDecimals(
            test.input,
            test.maxDigits,
            test.minDecimalPlaces,
            test.maxDecimalPlaces,
            test.removeTrailingZeros,
            test.allowExtraDecimalForLeadingZero);
        const std::string oldResult = limitDecimalsOriginal(
            test.input,
            test.maxDigits,
            test.minDecimalPlaces,
            test.maxDecimalPlaces,
            test.removeTrailingZeros,
            test.allowExtraDecimalForLeadingZero);

        if (result != test.expected) {
            ++currentFailures;
            std::cout << "limitDecimals failure: input=\"" << test.input << "\" expected=\""
                      << test.expected << "\" got=\"" << result << "\"\n";
        }
        if (oldResult != result) {
            ++oldCurrentMismatches;
            std::cout << "old/current mismatch: input=\"" << test.input << "\" old=\""
                      << oldResult << "\" current=\"" << result << "\"\n";
        }
    }

    std::cout << "limitDecimals failures: " << currentFailures << "\n";
    std::cout << "old/current mismatches: " << oldCurrentMismatches << "\n";
    if (currentFailures != 0 || oldCurrentMismatches != 0) {
        return 1;
    }

    constexpr int benchmarkIterations = 200000;
    auto benchmark = [&](auto formatter) {
        size_t checksum = 0;
        const auto start = std::chrono::steady_clock::now();
        for (int iteration = 0; iteration < benchmarkIterations; ++iteration) {
            for (const auto& test : cases) {
                const std::string result = formatter(test);
                checksum += result.size();
                checksum += result.empty() ? 0 : static_cast<unsigned char>(result.front());
            }
        }
        const auto end = std::chrono::steady_clock::now();
        return std::pair{
            std::chrono::duration<double, std::milli>(end - start).count(),
            checksum,
        };
    };

    const auto [oldMs, oldChecksum] = benchmark([](const LimitDecimalsCase& test) {
        return limitDecimalsOriginal(
            test.input,
            test.maxDigits,
            test.minDecimalPlaces,
            test.maxDecimalPlaces,
            test.removeTrailingZeros,
            test.allowExtraDecimalForLeadingZero);
    });
    const auto [currentMs, currentChecksum] = benchmark([](const LimitDecimalsCase& test) {
        return soemdsp::str::limitDecimals(
            test.input,
            test.maxDigits,
            test.minDecimalPlaces,
            test.maxDecimalPlaces,
            test.removeTrailingZeros,
            test.allowExtraDecimalForLeadingZero);
    });

    std::cout << "benchmark iterations: " << benchmarkIterations << " x " << cases.size() << " cases\n";
    std::cout << "original ms: " << oldMs << " checksum: " << oldChecksum << "\n";
    std::cout << "current ms: " << currentMs << " checksum: " << currentChecksum << "\n";
    std::cout << "winner: " << (currentMs < oldMs ? "current" : "original") << "\n";
    std::cout << "current/original ratio: " << (currentMs / oldMs) << "\n";
    return 0;
}
