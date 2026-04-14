#pragma once

#include "Wire.hpp"
#include "filter/Smoother.hpp"
#include "sehelper.hpp"

#include <algorithm>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace soemdsp::plugin {
using std::unordered_map;
using std::vector;
class SmootherManager {
  public:
    SE_CUSTOM_CONSTRUCTOR(SmootherManager)() {
        toSmooth_.reserve(128);
    }

    void sampleRateChanged() {
        for (auto& [id, obj] : smoothers_) {
            obj->incrementChanged();
        }
    }

    // Adds a smoother with given id, input only, and callback
    template<typename smoother_t>
    void registerSmoother(
      uint32_t id, double* ptrToTargetValue, std::function<void()> callback = []() {}) {
        DBG_ASSERT_ID_NOT_EXISTS(id);
        smoothers_[id]            = std::make_shared<smoother_t>();
        smoothers_[id]->callback_ = callback;
        smoothers_[id]->in_.pointTo(ptrToTargetValue);
    }

    // Adds a smoother with given id, output only, and callback
    template<typename smoother_t>
    void registerSmoother(
      uint32_t id, Wire<double>& pointVarToOutput, std::function<void()> callback = []() {}) {
        DBG_ASSERT_ID_NOT_EXISTS(id);
        smoothers_[id]            = std::make_shared<smoother_t>();
        smoothers_[id]->callback_ = callback;
        pointVarToOutput.pointTo(&smoothers_[id]->out_);
    }
    // Adds a smoother with given id, input, output, callback
    template<typename smoother_t>
    void registerSmoother(
      uint32_t id, double* ptrToTargetValue, Wire<double>& pointVarToOutput, std::function<void()> callback = []() {}) {
        DBG_ASSERT_ID_NOT_EXISTS(id);
        smoothers_[id]            = std::make_shared<smoother_t>();
        smoothers_[id]->callback_ = callback;
        smoothers_[id]->in_.pointTo(ptrToTargetValue);
        pointVarToOutput.pointTo(&smoothers_[id]->out_);
    }

    // Adds a smoother with an unused id starting at 10'000, returns the registered id
    template<typename smoother_t>
    uint32_t registerSmoother() {
        DBG_ASSERT_ID_NOT_EXISTS(autoId);
        smoothers_[autoId] = std::make_shared<smoother_t>();
        return autoId++;
    }

    void setInPtr(uint32_t id, double* ptrToTargetValue) {
        DBG_ASSERT_ID_EXISTS(id);
        smoothers_[id]->in_.pointTo(ptrToTargetValue);
    }
    void setTimeInSamplesPtr(uint32_t id, double* ptrToTimeInSamples) {
        DBG_ASSERT_ID_EXISTS(id);
        smoothers_[id]->timeInSamples_.pointTo(ptrToTimeInSamples);
    }
    void setTimeInSamplesPtrForAll(double* ptrToTimeInSamples) {
        for (const auto& [id, obj] : smoothers_) {
            obj->timeInSamples_.pointTo(ptrToTimeInSamples);
        }        
    }
    void setCallback(uint32_t id, std::function<void()> cb) {
        DBG_ASSERT_ID_EXISTS(id);
        smoothers_[id]->callback_ = cb;
    }

    // get smoother via id
    std::shared_ptr<filter::SmootherBase>& get(uint32_t id) {
        DBG_ASSERT_ID_EXISTS(id);
        return smoothers_[id];
    }

    double& outRef(uint32_t id) {
        DBG_ASSERT_ID_EXISTS(id);
        return smoothers_[id]->out_;
    }

    // Adds smoother for smoothing if it is not already added
    void addForSmoothing(uint32_t id, double v) {
        DBG_ASSERT_ID_EXISTS(id);
        auto& smoother = smoothers_[id];

        smoother->setTarget(v);

        // check if smoother was already added
        auto it = std::find_if(toSmooth_.begin(), toSmooth_.end(), [&smoother](const auto& ptr) { return ptr == smoother; });
        if (it == toSmooth_.end()) {
            toSmooth_.push_back(smoother);
        }
    }

    // Adds smoother for smoothing if it is not already added
    void addForSmoothing(uint32_t id) {
        DBG_ASSERT_ID_EXISTS(id);
        auto& smoother = smoothers_[id];

        smoother->targetChanged();

        // check if smoother was already added
        auto it = std::find_if(toSmooth_.begin(), toSmooth_.end(), [&smoother](const auto& ptr) { return ptr == smoother; });
        if (it == toSmooth_.end()) {
            toSmooth_.push_back(smoother);
        }
    }

    // Run this function per sample
    void run() {
        for (auto& o : toSmooth_) {
            o->run();
        }
    }

    // Run this function per block
    void clean() {
        toSmooth_.erase(std::remove_if(toSmooth_.begin(), toSmooth_.end(), [](const auto& obj) { return !obj->needsSmoothing(); }), toSmooth_.end());
    }

    // WARNING: does nothing if smoother does not exist for given id
    void snapValue(uint32_t id, double value) {
        DBG_ASSERT_ID_EXISTS(id);
        if (smoothers_.find(id) != smoothers_.end()) {
            smoothers_[id]->setState(value);
            smoothers_[id]->callback_();
        }
    }

    unordered_map<size_t, std::shared_ptr<filter::SmootherBase>> smoothers_;
    unordered_map<size_t, double> TimeInSamples_;
    vector<std::shared_ptr<filter::SmootherBase>> toSmooth_;

    // Overload the [] operator for non-const access.
    filter::SmootherBase& operator[](uint32_t id) {
        DBG_ASSERT_ID_EXISTS(id);
        return *smoothers_[id];
    }

template<typename enum_t>
    filter::SmootherBase& operator[](enum_t e_val) {
        DBG_ASSERT_ENUM_ID_EXISTS(e_val);
        return *smoothers_[Enum::sizeT(e_val)];
    }


  private:
    void DBG_ASSERT_ID_NOT_EXISTS(uint32_t id) {
        SE_ERROR(smoothers_.find(id) == smoothers_.end(), fmt::format("Smoother with ID {} was already registered.", id));
    }
    void DBG_ASSERT_ID_EXISTS(uint32_t id) {
        SE_ERROR(smoothers_.find(id) != smoothers_.end(), fmt::format("Smoother with ID {} does not exist.", id));
    }
    template<typename enum_t>
    void DBG_ASSERT_ENUM_ID_EXISTS(enum_t id) {
        SE_ERROR(smoothers_.find(Enum::sizeT(id)) != smoothers_.end(), fmt::format("Smoother with ID {} does not exist.", Enum::sizeT(id)));
    }
    uint32_t autoId{ 10'000 };
};
} // namespace soemdsp::plugin
