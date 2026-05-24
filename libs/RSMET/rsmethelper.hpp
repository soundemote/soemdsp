#pragma once

#include "soemdsp/sehelper.hpp"
#include "soemdsp/semath.hpp"
#include "Constants.h"
#include "TypeDefinitions.h"
#include "IntegerFunctions.h"
#include "GlobalDefinitions.h"
#include <string_view>
#include <complex>
#ifndef NDEBUG
inline void rsAssert(bool) {}
inline void rsAssert(bool, std::string_view) {}
inline void rsError(const char*) {}
#else
inline void rsAssert(bool check) {
    assert(check);
}
inline void rsAssert(bool check, std::string_view msg) {
    soemdsp::debug::CHECK(check, msg);
}
inline void rsError(const char* message = nullptr) {
    if (message) {
        soemdsp::debug::CHECK(false, message);
    } else {
        soemdsp::debug::CHECK(false, "Error");
    }
}
#endif
