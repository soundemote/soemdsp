#pragma once

#include <functional>
#include "sehelper.hpp"
#include "soemdsp/EnumArray.hpp"

namespace soemdsp {

//enum class UpdateFlags {
//    frequency,
//    pitch,
//    seed
//};

template<typename enum_t> 
struct DirtyUpdater {
    SE_DEFAULT_CONSTRUCTOR(DirtyUpdater);

    static constexpr size_t numFlags_{ Enum::count<enum_t>() };
    std::array<std::function<void()>, numFlags_> callback_{};
    std::array<bool, numFlags_> isDirty_{ true };

    void markDirty(enum_t flag) {
        isDirty_[Enum::sizeT(flag)] = true;
    }

    void markClean(enum_t flag) {
        isDirty_[Enum::sizeT(flag)] = false;
    }

    bool needsUpdate(enum_t flag) {
        return isDirty_[Enum::sizeT(flag)];
    }

    void update(enum_t flag) {
        callback_[Enum::sizeT(flag)]();
        markClean(flag);
    }

    void updateIfNeeded(enum_t flag) {
        if (needsUpdate(flag)) {
            update(flag);
        }
    }

    void updateAllIfNeeded() {
        for (auto color : magic_enum::enum_values<enum_t>()) {
            updateIfNeeded(color);
        }
    }
};

} // namespace soemdsp
