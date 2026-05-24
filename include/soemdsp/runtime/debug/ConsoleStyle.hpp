#pragma once

#include <ostream>
#include <string_view>

namespace soemdsp::runtime::debug::console
{

inline constexpr std::string_view reset{ "\033[0m" };
inline constexpr std::string_view dim{ "\033[2m" };
inline constexpr std::string_view green{ "\033[32m" };
inline constexpr std::string_view cyan{ "\033[36m" };
inline constexpr std::string_view yellow{ "\033[33m" };
inline constexpr std::string_view magenta{ "\033[35m" };
inline constexpr std::string_view gray{ "\033[90m" };

inline void title(std::ostream& out, std::string_view text)
{
    out << green << text << reset << "\n";
}

inline void section(std::ostream& out, std::string_view text)
{
    out << "\n"
        << cyan << "== " << text << " ==" << reset << "\n";
}

inline void divider(std::ostream& out)
{
    out << dim << "----------------------------------------" << reset << "\n";
}

inline void success(std::ostream& out, std::string_view text)
{
    out << green << "[ok] " << reset << text << "\n";
}

inline void info(std::ostream& out, std::string_view text)
{
    out << cyan << "[info] " << reset << text << "\n";
}

inline void debug(std::ostream& out, std::string_view text)
{
    out << magenta << "[dbg] " << reset << text << "\n";
}

} // namespace soemdsp::runtime::debug::console
