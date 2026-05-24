#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <soemdsp/utility/operations_string.hpp>

#include "parameterprototype.hpp"

namespace soemdsp::plugin {

class Parameter {
  public:
    Parameter() = default;

    template<typename Impl>
    Parameter(Impl impl)
      : model_(std::make_unique<Model<Impl>>(std::move(impl))) {}

    // Move Only
    Parameter(const Parameter&)                = delete; // No copying
    Parameter& operator=(const Parameter&)     = delete;
    Parameter(Parameter&&) noexcept            = default;
    Parameter& operator=(Parameter&&) noexcept = default;

    uint32_t id{}; // iterate over parameters to set id

    double getDspDouble() const { return model_->getDspDouble(); }
    float get() const { return static_cast<float>(getDspDouble()); }
    double getLinear() const { return model_->getLinear(); }
    double& getLinearRef() { return model_->getLinearRef(); }
    void set(double v) { model_->updateValueFromLinear(v); }

    std::string display() const { return model_->displayString(); }
    std::string getPatchValueString() const { return model_->getPatchValueString(); }
    void updateFromXml(const std::string& s) { model_->updateValueFromXml(s); }

  private:
    struct Concept {
        virtual ~Concept()                             = default;
        virtual std::unique_ptr<Concept> clone() const = 0;

        virtual double getDspDouble() const                   = 0;
        virtual double getLinear() const                      = 0;
        virtual double& getLinearRef()                        = 0;
        virtual void updateValueFromLinear(double v)          = 0;
        virtual std::string displayString() const             = 0;
        virtual std::string getPatchValueString() const       = 0;
        virtual void updateValueFromXml(const std::string& s) = 0;
    };

    template<typename T>
    struct Model final : Concept {
        T impl;

        Model(T t)
          : impl(std::move(t)) {}

        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model<T>>(impl);
        }

        double getDspDouble() const override { return impl.getDspDouble(); }
        double getLinear() const override { return impl.getLinear(); }
        double& getLinearRef() override { return impl.getLinearRef(); }
        void updateValueFromLinear(double v) override { impl.updateValueFromLinear(v); }
        std::string displayString() const override { return impl.displayString(); }
        std::string getPatchValueString() const override { return impl.getPatchValueString(); }
        void updateValueFromXml(const std::string& s) override { impl.updateValueFromXml(s); }
    };

    std::unique_ptr<Concept> model_;
};

} // namespace soemdsp::plugin
