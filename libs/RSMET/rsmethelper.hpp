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
#define RS_DEBUG_BREAK
#else
inline void rsAssert(bool check) {
    SE_ASSERT(check);
}
inline void rsAssert(bool check, std::string_view msg) {
    SE_ERROR(check, msg);
}
inline void rsError(const char* message = nullptr) {
    if (message) {
        SE_ERROR(false, message);
    } else {
        SE_ERROR(false, "Error");
    }
}
#define RS_DEBUG_BREAK se_assert(false);
#endif
