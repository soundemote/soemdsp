#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <soemdsp/utility/StringIterator.hpp>
#include <soemdsp/utility/operations_char.hpp>
#include <soemdsp/sehelper.hpp>

namespace soemdsp::str {
void removeWhiteSpace(std::string& s);
void removeNonDigits(std::string& s);
void removeTrailingZeros(std::string& s);
void replace(std::string& s, char matchChar, char replaceChar);
void padRight(std::string& s, std::string_view padding, int length);

std::vector<std::string> split(const std::string& s, std::string_view delimiter);
std::string limitDecimals(const std::string& v, int maxDigits, int minDecimalPlaces, int maxDecimalPlaces, bool removeTrailingZeros = false, bool allowExtraDecimalForLeadingZero = false);
std::string time(double seconds, int precision = 1);



} // namespace soemdsp::str

namespace soemdsp::convert {
inline double string_to_double(std::string_view v) {
    try {
        return std::stod(std::string(v));
    } catch (std::invalid_argument&) {
        debug::STOP();
    }
}
inline int string_to_int(std::string_view v) {
    try {
        return std::stoi(v.data());
    } catch (std::invalid_argument&) {
        debug::STOP();
    }
}
} //namespace soemdsp::convert

// string a;
// a = str::limitDecimals("1.999995", 3, 2, 5, false);
// debug::CHECK(a == "2.00");
// a = str::limitDecimals("9.999995", 3, 2, 5, false);
// debug::CHECK(a == "10.0");
// a = str::limitDecimals("9999.99995", 3, 2, 5, false);
// debug::CHECK(a == "10000");
// a = str::limitDecimals("9.999995", 3, 4, 5, true);
// debug::CHECK(a == "10");
// a = str::limitDecimals("9.999995", 4, 4, 5, false);
// debug::CHECK(a == "10.00");
// a = str::limitDecimals("0.999995", 4, 4, 5, false);
// debug::CHECK(a == "1.000");
// a = str::limitDecimals(" +.987655", 3, 2, 5, true);
// debug::CHECK(a == "+0.99");
// a = str::limitDecimals(" + .987655", 3, 2, 5, true);
// debug::CHECK(a == "+0");
// a = str::limitDecimals(" 46 .987655", 3, 2, 5, true);
// debug::CHECK(a == "46");
// a = str::limitDecimals(" abc .987655", 3, 2, 5, true);
// debug::CHECK(a == "0");
// a = str::limitDecimals("abc.987655", 3, 2, 5, true);
// debug::CHECK(a == "0");
// a = str::limitDecimals("1.987655", 3, 2, 5, true);
// debug::CHECK(a == "1.99");
// a = str::limitDecimals("0.987655", 3, 3, 3, false, true);
// debug::CHECK(a == ".988");
// a = str::limitDecimals("21.987655", 3, 3, 3, false, true);
// debug::CHECK(a == "22.0");
// a = str::limitDecimals("321.987655", 3, 3, 3, false, true);
// debug::CHECK(a == "322");
// a = str::limitDecimals("321.1234", 3, 3, 3, false, true);
// debug::CHECK(a == "321");
// a = str::limitDecimals("21.987655", 3, 2, 5, false);
// debug::CHECK(a == "22.0");
// a = str::limitDecimals("21.987655", 3, 2, 5, true);
// debug::CHECK(a == "22");
// a = str::limitDecimals("321.987655", 3, 2, 5, true);
// debug::CHECK(a == "322");
// a = str::limitDecimals("1.4444444", 3, 2, 5, false);
// debug::CHECK(a == "1.44");
// a = str::limitDecimals("9.4444444", 3, 2, 5, false);
// debug::CHECK(a == "9.44");
// a = str::limitDecimals("9999.4444444", 3, 2, 5, false);
// debug::CHECK(a == "9999");
// a = str::limitDecimals("9.4444444", 3, 4, 5, true);
// debug::CHECK(a == "9.44");
// a = str::limitDecimals("9.4444444", 4, 4, 5, false);
// debug::CHECK(a == "9.444");
// a = str::limitDecimals("0.4444444", 4, 4, 5, false);
// debug::CHECK(a == "0.444");
// a = str::limitDecimals(" +.4444444", 3, 2, 5, true);
// debug::CHECK(a == "+0.44");
// a = str::limitDecimals("1.4444444", 3, 2, 5, true);
// debug::CHECK(a == "1.44");
// a = str::limitDecimals("21.4444444", 3, 2, 5, false);
// debug::CHECK(a == "21.4");
// a = str::limitDecimals("21.4444444", 3, 2, 5, true);
// debug::CHECK(a == "21.4");
// a = str::limitDecimals("321.4444444", 3, 2, 5, true);
// debug::CHECK(a == "321");
