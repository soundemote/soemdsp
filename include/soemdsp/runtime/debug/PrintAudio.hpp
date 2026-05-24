#pragma once

#include <cstddef>
#include <iostream>
#include <string_view>

namespace soemdsp::runtime::debug
{

inline void printAudio(std::string_view label,
                       const float* buffer,
                       std::size_t count)
{
    std::cout << "[" << label << "] ";

    if (!buffer)
    {
        std::cout << "<null>\n";
        return;
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        std::cout << buffer[i] << " ";
    }

    std::cout << "\n";
}

} // namespace soemdsp::runtime::debug
