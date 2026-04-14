#pragma once

#include <string_view>

using std::string_view;

namespace soemdsp::cha {
// check if character is uppercase
bool isUpper(char c);
// check if character is lowercase
bool isLower(char c);
// convert character to uppercase
char toUpper(char c);
// convert character to lowercase
char toLower(char c);
// compare two characters
bool is(char c1, char c2);
// case-insensitive compare two characters
bool is_i(char c1, char c2);
// check if character is found in string
bool isAny(char c, std::string_view list);
// case-insensitive check if character is found in string
bool isAny_i(char c, std::string_view list);
// check for a whitespace character
bool isWhiteSpace(char c);
// check for a newline character
bool isNewLine(char c);
// check if character is any of 0123456789
bool isDigit(char c);
// check if character is any of +-.0123456789
bool isNumeric(char c);
// check if character is any of capitol or lowercase ABCDEFGHIJKLMNOPQRSTUVWXYZ
bool isAlphabet(char c);
// Translates 0123456789 char digit to its value, returns 0 if not a digit.
int numericValue(char c);
} // namespace soemdsp::cha
