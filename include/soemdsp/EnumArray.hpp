#pragma once

#include <magic_enum.hpp>

#include <array>

/**
 * @brief Array with enumerated indexes
 * It's either this or removing the "class" from "enum class"
 * @tparam T Element type within the array.
 * @tparam Enum Names for the array indexes.
 */
template<typename t, typename enum_t>
struct EnumArray : public std::array<t, magic_enum::enum_count<enum_t>()> {
    using array_t = std::array<t, magic_enum::enum_count<enum_t>()>;

    t& operator[](enum_t e_val) {
        return array_t::operator[](static_cast<typename array_t::size_type>(e_val));
    }

    const t& operator[](enum_t e_val) const {
        return array_t::operator[](static_cast<typename array_t::size_type>(e_val));
    }

    t& operator[](size_t i_val) { return array_t::operator[](static_cast<typename array_t::size_type>(i_val)); }

    const t& operator[](size_t i_val) const {
        return array_t::operator[](static_cast<typename array_t::size_type>(i_val));
    }
};
