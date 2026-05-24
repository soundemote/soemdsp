#pragma once

#include <functional>

#include <soemdsp/utility/EnumArray.hpp>
#include <soemdsp/sehelper.hpp>

namespace soemdsp {
template<typename enum_t>
struct DirtyUpdater {
    DirtyUpdater() = default;

    static constexpr size_t numFlags_{ magic_enum::enum_count<enum_t>() };
    std::array<std::function<void()>, numFlags_> callback_{};
    std::array<bool, numFlags_> isDirty_{ true };

    inline void markDirty(enum_t flag) noexcept {
        isDirty_[magic_enum::enum_index(flag)] = true;
    }

    inline void markClean(enum_t flag) noexcept {
        isDirty_[magic_enum::enum_index(flag)] = false;
    }

    // ensures this is used in if statements
    [[nodiscard]] inline bool needsUpdate(enum_t flag) const noexcept {
        return isDirty_[magic_enum::enum_index(flag)];
    }

    inline void update(enum_t flag) {
        callback_[magic_enum::enum_index(flag)]();
        markClean(flag);
    }

    inline void updateIfNeeded(enum_t flag) {
        if (needsUpdate(flag)) {
            update(flag);
        }
    }

    inline void updateAllIfNeeded() {
        for (auto flag : magic_enum::enum_values<enum_t>()) {
            updateIfNeeded(flag);
        }
    }
};

} // namespace soemdsp
