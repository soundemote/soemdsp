#include "operations_char.hpp"

#include <cctype>

using std::string_view;
using std::toupper;
using std::tolower;
using std::isdigit;
using std::isupper;
using std::islower;
using std::isalpha;
using std::isspace;

namespace soemdsp::cha {
char toUpper(char c) {
    return toupper(static_cast<unsigned char>(c));
}
char toLower(char c) {
    return tolower(static_cast<unsigned char>(c));
}
bool is(char c1, char c2) {
    return c1 == c2;
}
bool is_i(char c1, char c2) {
    return toLower(c1) == toLower(c2);
}
bool isDigit(char c) {
    return isdigit(c);
}
bool isNumeric(char c) {
    return isdigit(c) || c == '+' || c == '-' || c == '.';
}
bool isAlphabet(char c) {
    return isalpha(c) != 0;
}
bool isWhiteSpace(char c) {
    return isspace(c) != 0;
}
bool isNewLine(char c) {
    return isAny(c, "\r\n");
}

bool isAny(char c, string_view list) {
    return list.find_first_of(c) != std::string::npos;
}
bool isAny_i(char c, string_view list) {
    for (auto iter = list.begin(); iter != list.end(); ++iter) {
        if (is_i(c, *iter)) {
            return true;
        }
    }
    return false;
}
bool isUpper(char c) {
    return isupper(c);
}
bool isLower(char c) {
    return islower(c);
}
int numericValue(char c) {
    if (!isDigit(c))
        return 0;

    return int(c) - '0';
}
} // namespace soemdsp::cha
