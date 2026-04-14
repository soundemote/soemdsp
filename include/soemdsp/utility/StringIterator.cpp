#include "StringIterator.hpp"

#include "soemdsp/operations_vector.hpp"

using std::string;
using std::string_view;
using std::distance;
using std::numeric_limits;

namespace soemdsp::str {
void StringIterator::restart() {
    size_ = 0;
    len   = in_str.length();
    pos   = 0;
    p1    = in_str.begin();
    p2    = p1;
}
void StringIterator::toEnd() {
    pos = len - 1;
    p2  = in_str.end() - 1;
}

// have p1 = p2, used before doing a series of p2 increments
void StringIterator::start() {
    p1 = p2;
}

// increment p2 iterator without returning character
void StringIterator::move(int i) {
    pos += i;
    p2 += i;
}
void StringIterator::back(int i) {
    pos -= i;
    p2 -= i;
}

void StringIterator::remember() {
    if (p2 > p1) {
        memory.push_back({ p1, p2 });
        size_ += abs(distance(p1, p2));
    } else {
        memory.push_back({ p2, p1 });
        size_ += distance(p2, p1);
    }
}

string StringIterator::popMemory() {
    string s;
    s.reserve(size_);

    for (const auto& mem : memory) {
        s += string(mem.p1, mem.p2);
    }

    memory.clear();
    size_ = 0;

    return s;
}

void StringIterator::moveTo(char c) {
    while (!atEnd() && !is(c)) {
        move();
    }
}
void StringIterator::movePast(char c, int n) {
    n     = (n == 0 ? numeric_limits<int>::max() : n);
    int i = 0;
    while (i < n && !atEnd() && is(c)) {
        move();
        ++i;
    }
}

void StringIterator::consume(char c, int n) {
    n = (n == 0 ? numeric_limits<int>::max() : n);
    start();
    int i = 0;
    while (i < n && !atEnd() && is(c)) {
        move();
        ++i;
    }
    remember();
}

void StringIterator::backTo(char c) {
    while (!atBegin() && !is(c)) {
        back();
    }
}
void StringIterator::backPast(char c) {
    while (!atBegin() && is(c)) {
        back();
    }
}

void StringIterator::backConsume(char c, int n) {
    n = (n == 0 ? numeric_limits<int>::max() : n);
    start();
    int i = 0;
    while (i < n && !atBegin() && is(c)) {
        back();
        ++i;
    }
    remember();
}

void StringIterator::moveTo(string_view c) {
    while (!atEnd() && !is(c)) {
        move();
    }
}
void StringIterator::movePast(string_view c, int n) {
    n     = (n == 0 ? numeric_limits<int>::max() : n);
    int i = 0;
    while (i < n && !atEnd() && is(c)) {
        move();
        ++i;
    }
}
void StringIterator::consume(string_view c, int n) {
    n = (n == 0 ? numeric_limits<int>::max() : n);
    start();
    int i = 0;
    while (i < n && !atEnd() && is(c)) {
        move();
        ++i;
    }
    remember();
}
void StringIterator::backTo(string_view c) {
    while (!atBegin() && !is(c)) {
        back();
    }
}
void StringIterator::backConsume(string_view c, int n) {
    n = (n == 0 ? numeric_limits<int>::max() : n);
    start();
    int i = 0;
    while (i < n && !atBegin() && is(c)) {
        back();
        ++i;
    }
    remember();
}

void StringIterator::moveToNewLine() {
    while (!atEnd() && !is("\r\n")) {
        move();
    }
}
void StringIterator::movePastNewLine() {
    while (!atEnd()) {
        if (is('\n')) {
            move();
        } else if (is('\r')) {
            move();
            if (is('\n')) {
                move();
            }
        } else {
            return;
        }
    }
}
void StringIterator::consumeNewLine(int n) {
    n     = (n == 0 ? numeric_limits<int>::max() : n);
    int i = 0;
    start();
    while (i < n && !atEnd()) {
        if (is('\n')) {
            move();
        } else if (is('\r')) {
            move();
            if (is('\n')) {
                move();
            }
        } else {
            return;
        }
        ++i;
    }
    remember();
}

void StringIterator::consumeSpace(int n) {
    n     = (n == 0 ? numeric_limits<int>::max() : n);
    int i = 0;
    start();
    while (i < n && !atEnd()) {
        if (is('\n')) {
            move();
        } else if (is('\r')) {
            move();
            if (is('\n')) {
                move();
            }
        } else if (is(SPACES)) {
            move();
        } else {
            return;
        }
        ++i;
    }
}

bool StringIterator::atEnd() const {
    return pos >= len;
}
bool StringIterator::atBegin() const {
    return pos <= 0;
}
bool StringIterator::is(char c) const {
    return *p2 == c;
}
bool StringIterator::is(string_view c) const {
    return cha::isAny(*p2, c);
}

// void StringIterator::moveToSpaceOrChar(char c) {
//     while (!atEnd() && *this != c && !cha::isWhiteSpace(*this))
//         move();
// }
//

// string StringIterator::consumeToSequenceAndSkip(const vector<string>& sequences, int* sequenceFound) {
//     while (!atEnd()) {
//         for (int i = 0; i < sequences.size(); ++i) {
//             if (lookAheadForSequence(sequences[i])) {
//                 remember();
//                 skip(sequences[i].length());
//                 if (sequenceFound != nullptr)
//                     *sequenceFound = i;
//                 start();
//                 return popMemory();
//             }
//         }
//
//         move();
//     }
// }
//
// string StringIterator::consumeToSequence(const vector<string>& sequences, int* sequenceFound) {
//     start();
//
//     while (!atEnd()) {
//         for (int i = 0; i < sequences.size(); ++i) {
//             if (lookAheadForSequence(sequences[i])) {
//                 if (sequenceFound != nullptr)
//                     *sequenceFound = i;
//
//                 return get();
//             }
//         }
//
//         skip();
//     }
//
//     if (sequenceFound != nullptr)
//         *sequenceFound = -1;
//     return get();
// }
//
// string StringIterator::consumePastSequence(const vector<string>& sequences, int* sequenceFound) {
//     while (!atEnd()) {
//         for (int i = 0; i < sequences.size(); ++i) {
//             if (lookAheadForSequence(sequences[i])) {
//                 if (sequenceFound != nullptr)
//                     *sequenceFound = i;
//
//                 skip(sequences[i].length());
//
//                 return get();
//             }
//         }
//
//         move();
//     }
//
//     return get();
// }
//
// string StringIterator::consumePastSequence(const string& sequence) {
//     return consumePastSequence(vector<string>{ sequence });
// }
//
// bool StringIterator::lookAheadForSequence(const string& sequence) {
//     int l        = sequence.length();
//     int posInSeq = 0;
//
//     auto p = p2;
//
//     while (posInSeq < l && *p == sequence[posInSeq]) {
//         ++posInSeq;
//         ++p;
//     }
//
//     return posInSeq == l;
// }
//
// string StringIterator::prepareForNaturalSort() {
//     enum MODE {
//         skipSpaces,
//         zeroFound,
//         consume
//     };
//     MODE mode = skipSpaces;
//
//     while (!atEnd()) {
//         switch (mode) {
//         case skipSpaces:
//             if (isAnyOf(" _")) {
//                 move();
//             } else if (is('0')) {
//                 move();
//                 mode = zeroFound;
//             } else {
//                 start();
//                 move();
//                 mode = consume;
//             }
//             continue;
//         case zeroFound:
//             if (is('0')) {
//                 move();
//             } else if (isDigit()) {
//                 start();
//                 move();
//                 mode = consume;
//             } else {
//                 start();
//                 back();
//                 if (isAnyOf("_ ")) {
//                     remember();
//                     move();
//                     start();
//                     mode = skipSpaces;
//                 } else {
//                     move();
//                     mode = consume;
//                 }
//             }
//             continue;
//         case consume:
//             if (!isAnyOf(" _")) {
//                 move();
//             } else {
//                 remember();
//                 move();
//                 start();
//                 mode = skipSpaces;
//             }
//             continue;
//         }
//     }
//     remember();
//     return popMemory();
// }
//
// string StringIterator::consumeToChar(char c) {
//     start();
//     while (!atEnd() && *this != c)
//         move();
//     return get();
// }
//
// string StringIterator::consumeToChar(const string& s) {
//     start();
//     while (!atEnd() && !isAnyOf(s))
//         move();
//     return get();
// }
//
// string StringIterator::consumePastChar(const string& s) {
//     start();
//     while (!atEnd() && !isAnyOf(s))
//         move();
//     while (!atEnd() && isAnyOf(s))
//         move();
//     return get();
// }
//
// string StringIterator::consumeToSpaceOrChar(char c) {
//     start();
//     skipToSpaceOrChar(c);
//     return get();
// }
//
// string StringIterator::consumeToEnd(int stop_early) {
//     start();
//     while (!atEnd(stop_early))
//         move();
//     return get();
// }
//
// string StringIterator::consumeToNewLineTRIM() {
//     skipSpaces();
//     auto p = p1 = p2;
//     while (!atEnd() && !cha::isNewLine(*this)) {
//         skipToSpace();
//         p = p2;
//         skipChar(' ');
//         if (cha::isNewLine(*this))
//             break;
//     }
//     return string(p1, p);
// }
//
// string StringIterator::consumeToCharTRIM(char c) {
//     skipSpaces();
//     auto p = p1 = p2;
//     while (!atEnd() && *this != c) {
//         skipToSpaceOrChar(c);
//         p = p2;
//         skipSpaces();
//         if (*this == c)
//             break;
//     }
//     return string(p1, p);
// }
//
// string StringIterator::consumeToNewlineOrCharTRIM(char c) {
//     skipSpaces();
//     auto p = p1 = p2;
//     while (!atEnd() && *this != c) {
//         skipToSpaceOrChar(c);
//         p = p2;
//         skipChar(' ');
//         if (*this == c || cha::isNewLine(*this))
//             break;
//     }
//     return string(p1, p);
// }
//
// string StringIterator::consumeToChar_IgnoreEscapes(const string& s, char escape) {
//     start();
//
//     while (!atEnd() && !cha::isAnyOf(*this, s)) {
//         if (*this == escape) {
//             consumeAndSkipOver();
//             skip();
//             continue;
//         }
//
//         move();
//     }
//
//     remember();
//
//     return popMemory();
// }
//  string StringIterator::convertToWhiteSpace(const string& s) {
//     StringIterator si(s);
//
//     enum {
//         newline2,
//         newline1,
//         tab
//     };
//     vector<string> sequences({ "\r\n", "\n", "\t" });
//     vector<string> found;
//
//     while (!si.atEnd()) {
//         int sequenceFound;
//         found.push_back(replaceAll(si.consumeToSequence(sequences, &sequenceFound), ' '));
//         switch (sequenceFound) {
//         case newline2:
//             found.push_back("\r\n");
//             si.skip(2);
//             break;
//         case newline1:
//             found.push_back("\n");
//             si.skip();
//             break;
//         case tab:
//             found.push_back("\t");
//             si.skip();
//             break;
//         }
//     }
//
//     return VEC::toString(found);
// }
//
//  string StringIterator::replaceAll(const string& s, juce_wchar c) {
//      return string::repeatedString(string::charToString(c), s.length());
//  }
} // namespace soemdsp::str
