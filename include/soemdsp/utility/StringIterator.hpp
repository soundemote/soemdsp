#pragma once

#include "soemdsp/operations_char.hpp"
#include "soemdsp/sehelper.hpp"

#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace soemdsp::str {
const std::string SPACES        = " \t";
const std::string LINES         = "\r\n";
const std::string BLANKS        = " \t\r\n";
const std::string DIGITS        = "0123456789";
const std::string NONZERODIGITS = "123456789";
const std::string ALPHADIGITS   = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string SIGNS         = "+-";
const std::string NUMERICS      = "+-.0123456789";
const std::string ALPHANUMERICS = "+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* WARNING: All functions need to be tested after migration, things will definitely be working very horribly wrong!!! */
class StringIterator {
  public:
    // static std::string convertToWhiteSpace(const std::string& s);

    // static string replaceAll(const string& s, char c);

    StringIterator(const std::string& str)
      : in_str(str)
      , p1(str.begin())
      , p2(str.begin()) {
        SE_ASSERT(!str.empty());
        restart();
    }

    // operator
    // char operator++() {
    //    return inc();
    //}
    // char operator[](int i) const {
    //    return in_str[i];
    //}
    //// void operator=(const std::string& s) { in_str = s; restart(); }

    //// conversion
    // operator const int() const {
    //     return pos;
    // }
    // operator const char() const {
    //     return *p2;
    // }
    // operator const std::string() const {
    //     return in_str;
    // }

    //// comparison
    // bool operator!=(char c) const {
    //     return *p2 != c;
    // }
    // bool operator==(char c) const {
    //     return *p2 == c;
    // }
    // bool operator<(char c) const {
    //     return *p2 < c;
    // }
    // bool operator>(char c) const {
    //     return *p2 > c;
    // }

    // bool operator!=(int i) const {
    //     return pos != i;
    // }
    // bool operator==(int i) const {
    //     return pos == i;
    // }
    // bool operator<(int i) const {
    //     return pos < i;
    // }
    // bool operator>(int i) const {
    //     return pos > i;
    // }

    std::string::const_iterator p1; // behind iterator
    std::string::const_iterator p2; // ahead iterator

    size_t size() {
        return size_;
    }

    // restart iterator back to beginning of std::string
    void restart();

    // set ahead iterator to end for back functions
    void toEnd();

    // have p1 = p2, used before doing a series of p2 increments
    void start();

    // increment p2 iterator without returning character, n is the number of times to perform the action
    void move(int i = 1);

    // decrement p2 iterator without returning character, n is the number of times to perform the action
    void back(int i = 1);

    // Add a substring to memory
    void remember();
    int getLastMemoryLength() {
        return toInt(abs(std::distance(memory.back().p1, memory.back().p2)));
    }

    // Return all remembered substrings as a single std::string and clear memory.
    std::string popMemory();

    void moveTo(char c);
    void movePast(char c, int n = 0);
    void consume(char c, int n);
    void backTo(char c);
    void backPast(char c);
    void backConsume(char c, int n = 0);

    void moveTo(std::string_view c);
    void movePast(std::string_view c, int n = 0);
    void consume(std::string_view c, int n = 0);
    void backTo(std::string_view c);
    void backConsume(std::string_view c, int n = 0);

    void moveToNewLine();
    void movePastNewLine();
    void consumeNewLine(int n = 0);

    void consumeSpace(int n = 0);

    bool atEnd() const;
    bool atBegin() const;

    bool is(char c) const;
    bool is(std::string_view s) const;

    //// Add a substring to memory, skip next char, and have p1 = p2 which essentially removes previous characters from string. This is used to
    //// efficiently remove a single character (the next character) from the substring, especially useful for when trying to find and remove escape
    //// characters
    // void consumeAndSkipOver();

    // std::string consumeToSequenceAndSkip(const vector<std::string>& sequences, int* sequenceFound = nullptr);
    // std::string consumeToSequence(const vector<std::string>& sequences, int* sequenceFound = nullptr);
    // std::string consumePastSequence(const vector<std::string>& sequences, int* sequenceFound = nullptr);
    // std::string consumePastSequence(const std::string& sequence);
    // bool lookAheadForSequence(const std::string& sequence);
    // std::string prepareForNaturalSort();

    //// increments to end minus stop_early position and returns the substring
    // std::string consumeToEnd(int stop_early = 0);

    //// increments until encountering char and returns the substring
    // std::string consumeToChar(char c);

    //// increments until encountering any char in string and returns the substring
    // std::string consumeToChar(const std::string& s);

    //// increments until encountering any char in string, then increments until char is not encountered, and returns the substring
    // std::string consumePastChar(const std::string& s);

    //// increments until encountering char or white space and returns the substring
    // std::string consumeToSpaceOrChar(char c);

    //// increments until the last new line character and returns the substring without pre and post white space
    // std::string consumeToNewLineTRIM();

    //// increments until encounting char and returns the substring without pre and post white space
    // std::string consumeToCharTRIM(char c);

    //// increments until encounting char or newline and returns the substring without pre and post white space
    // std::string consumeToNewlineOrCharTRIM(char c);

    // std::string consumeToChar_IgnoreEscapes(const std::string& s, char escape);

    // std::string getCharsAhead(int length) {
    //     return in_str.substr(pos, pos + length);
    // }

  private:
    // private members

    // used to tell if we are at end of string or not;
    int pos{};
    int len{};
    size_t size_{}; // total size of all remembered strings
    const std::string& in_str;

    struct sub_mem {
        std::string::const_iterator p1;
        std::string::const_iterator p2;
    };

    std::vector<sub_mem> memory;

    // private functions

    // get substring of from iterators p1 and p2
    std::string get() {
        return std::string(p1, p2);
    }
};
} // namespace soemdsp::str
