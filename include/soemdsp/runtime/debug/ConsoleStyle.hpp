#pragma once

#include <ostream>
#include <string_view>

namespace soemdsp::runtime::debug::console
{

inline bool stylesEnabled{ true };

inline constexpr std::string_view reset{ "\033[0m" };
inline constexpr std::string_view dim{ "\033[2m" };
inline constexpr std::string_view green{ "\033[32m" };
inline constexpr std::string_view cyan{ "\033[36m" };
inline constexpr std::string_view yellow{ "\033[33m" };
inline constexpr std::string_view magenta{ "\033[35m" };
inline constexpr std::string_view gray{ "\033[90m" };

inline void setEnabled(bool enabled)
{
    stylesEnabled = enabled;
}

inline bool enabled()
{
    return stylesEnabled;
}

inline std::string_view style(std::string_view value)
{
    return stylesEnabled ? value : std::string_view{};
}

inline void title(std::ostream& out, std::string_view text)
{
    out << style(green) << text << style(reset) << "\n";
}

inline void section(std::ostream& out, std::string_view text)
{
    out << "\n"
        << style(cyan) << "== " << text << " ==" << style(reset) << "\n";
}

inline void divider(std::ostream& out)
{
    out << style(dim) << "----------------------------------------" << style(reset) << "\n";
}

inline void success(std::ostream& out, std::string_view text)
{
    out << style(green) << "[ok] " << style(reset) << text << "\n";
}

inline void info(std::ostream& out, std::string_view text)
{
    out << style(cyan) << "[info] " << style(reset) << text << "\n";
}

inline void debug(std::ostream& out, std::string_view text)
{
    out << style(magenta) << "[dbg] " << style(reset) << text << "\n";
}

} // namespace soemdsp::runtime::debug::console
