#include <iomanip>
#include <iostream>

#include <soemdsp/runtime/control/EvaluateControlGraph.hpp>
#include <soemdsp/runtime/control/ControlGraphParameterMidpoint.hpp>

using namespace soemdsp::runtime;

namespace
{

void printRow(float input)
{
    constexpr float midpoint = 0.25f;

    std::cout << std::setw(5) << input
              << " | "
              << std::setw(6)
              << evaluateControlCurveShape(ControlCurveShape::Linear, input)
              << " | "
              << std::setw(7)
              << evaluateControlCurveShape(ControlCurveShape::EaseIn, input)
              << " | "
              << std::setw(8)
              << evaluateControlCurveShape(ControlCurveShape::EaseOut, input)
              << " | "
              << std::setw(10)
              << evaluateControlCurveShape(ControlCurveShape::Smoothstep, input)
              << " | "
              << std::setw(8)
              << evaluateControlCurveShape(
                   ControlCurveShape::Midpoint,
                   input,
                   midpoint)
              << "\n";
}

Parameter makeCutoffParameter()
{
    Parameter parameter;
    parameter.id       = "cutoff";
    parameter.name     = "Cutoff";
    parameter.minValue = 20.0f;
    parameter.midValue = 1000.0f;
    parameter.maxValue = 20000.0f;
    return parameter;
}

void printParameterMidpointCurve()
{
    const auto parameter = makeCutoffParameter();
    const auto normalizedMidpoint =
      normalizedMidpointForParameter(parameter);

    ControlNode curveNode;
    curveNode.kind = ControlNodeKind::Curve;
    curveNode.name = "Cutoff midpoint curve";

    const auto applied =
      applyParameterMidpointToCurveNode(curveNode, parameter);

    std::cout << "\n[PARAMETER MIDPOINT CURVE]\n"
              << "normalized midpoint: ";

    if (normalizedMidpoint.has_value())
    {
        std::cout << *normalizedMidpoint << "\n";
    }
    else
    {
        std::cout << "<none>\n";
    }

    std::cout << "applied: "
              << (applied ? "true" : "false")
              << "\n";

    if (curveNode.curveSettings.has_value())
    {
        std::cout << "curve midpoint: "
                  << curveNode.curveSettings->midpoint
                  << "\n";
    }
}

} // namespace

int main()
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "input | linear | ease_in | ease_out | smoothstep | midpoint\n";
    std::cout << "------+--------+---------+----------+------------+---------\n";

    printRow(0.0f);
    printRow(0.25f);
    printRow(0.5f);
    printRow(0.75f);
    printRow(1.0f);
    printParameterMidpointCurve();

    return 0;
}
