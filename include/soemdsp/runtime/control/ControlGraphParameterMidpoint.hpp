#pragma once

#include <optional>
#include <soemdsp/runtime/Parameter.hpp>
#include <soemdsp/runtime/control/ControlGraph.hpp>

namespace soemdsp::runtime
{

inline std::optional<float> normalizedMidpointForParameter(
  const Parameter& parameter)
{
    if (parameter.maxValue == parameter.minValue)
    {
        return std::nullopt;
    }

    const auto normalized =
      (parameter.midValue - parameter.minValue) /
      (parameter.maxValue - parameter.minValue);

    if (normalized <= 0.0f || normalized >= 1.0f)
    {
        return std::nullopt;
    }

    return normalized;
}

inline bool applyParameterMidpointToCurveNode(
  ControlNode& node,
  const Parameter& parameter)
{
    if (node.kind != ControlNodeKind::Curve)
    {
        return false;
    }

    const auto normalizedMidpoint =
      normalizedMidpointForParameter(parameter);
    if (!normalizedMidpoint.has_value())
    {
        return false;
    }

    node.curveSettings = ControlCurveSettings{
      ControlCurveShape::Midpoint,
      *normalizedMidpoint };
    return true;
}

} // namespace soemdsp::runtime
