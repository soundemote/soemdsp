#pragma once

#include "operations_char.hpp"
#include "utility/StringIterator.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace soemdsp::str {
using std::clamp;
using std::max;
using std::min;
using std::string;
using std::string_view;
using std::stringstream;
using std::vector;

void removeWhiteSpace(string& s);
void removeNonDigits(string& s);
void removeTrailingZeros(string& s);
void replace(string& s, char matchChar, char replaceChar);
void padRight(string& s, string_view padding, int length);

vector<string> split(const string& s, string_view delimiter);
string limitDecimals(const string& v, int maxDigits, int minDecimalPlaces, int maxDecimalPlaces, bool removeTrailingZeros = false, bool allowExtraDecimalForLeadingZero = false);
string time(double seconds, int precision = 1);
} // namespace soemdsp::str

// string a;
// a = str::limitDecimals("1.999995", 3, 2, 5, false);
// SE_ASSERT(a == "2.00");
// a = str::limitDecimals("9.999995", 3, 2, 5, false);
// SE_ASSERT(a == "10.0");
// a = str::limitDecimals("9999.99995", 3, 2, 5, false);
// SE_ASSERT(a == "10000");
// a = str::limitDecimals("9.999995", 3, 4, 5, true);
// SE_ASSERT(a == "10");
// a = str::limitDecimals("9.999995", 4, 4, 5, false);
// SE_ASSERT(a == "10.00");
// a = str::limitDecimals("0.999995", 4, 4, 5, false);
// SE_ASSERT(a == "1.000");
// a = str::limitDecimals(" +.987655", 3, 2, 5, true);
// SE_ASSERT(a == "+0.99");
// a = str::limitDecimals(" + .987655", 3, 2, 5, true);
// SE_ASSERT(a == "+0");
// a = str::limitDecimals(" 46 .987655", 3, 2, 5, true);
// SE_ASSERT(a == "46");
// a = str::limitDecimals(" abc .987655", 3, 2, 5, true);
// SE_ASSERT(a == "0");
// a = str::limitDecimals("abc.987655", 3, 2, 5, true);
// SE_ASSERT(a == "0");
// a = str::limitDecimals("1.987655", 3, 2, 5, true);
// SE_ASSERT(a == "1.99");
// a = str::limitDecimals("0.987655", 3, 3, 3, false, true);
// SE_ASSERT(a == ".988");
// a = str::limitDecimals("21.987655", 3, 3, 3, false, true);
// SE_ASSERT(a == "22.0");
// a = str::limitDecimals("321.987655", 3, 3, 3, false, true);
// SE_ASSERT(a == "322");
// a = str::limitDecimals("321.1234", 3, 3, 3, false, true);
// SE_ASSERT(a == "321");
// a = str::limitDecimals("21.987655", 3, 2, 5, false);
// SE_ASSERT(a == "22.0");
// a = str::limitDecimals("21.987655", 3, 2, 5, true);
// SE_ASSERT(a == "22");
// a = str::limitDecimals("321.987655", 3, 2, 5, true);
// SE_ASSERT(a == "322");
// a = str::limitDecimals("1.4444444", 3, 2, 5, false);
// SE_ASSERT(a == "1.44");
// a = str::limitDecimals("9.4444444", 3, 2, 5, false);
// SE_ASSERT(a == "9.44");
// a = str::limitDecimals("9999.4444444", 3, 2, 5, false);
// SE_ASSERT(a == "9999");
// a = str::limitDecimals("9.4444444", 3, 4, 5, true);
// SE_ASSERT(a == "9.44");
// a = str::limitDecimals("9.4444444", 4, 4, 5, false);
// SE_ASSERT(a == "9.444");
// a = str::limitDecimals("0.4444444", 4, 4, 5, false);
// SE_ASSERT(a == "0.444");
// a = str::limitDecimals(" +.4444444", 3, 2, 5, true);
// SE_ASSERT(a == "+0.44");
// a = str::limitDecimals("1.4444444", 3, 2, 5, true);
// SE_ASSERT(a == "1.44");
// a = str::limitDecimals("21.4444444", 3, 2, 5, false);
// SE_ASSERT(a == "21.4");
// a = str::limitDecimals("21.4444444", 3, 2, 5, true);
// SE_ASSERT(a == "21.4");
// a = str::limitDecimals("321.4444444", 3, 2, 5, true);
// SE_ASSERT(a == "321");
