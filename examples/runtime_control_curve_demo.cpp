#include <iomanip>
#include <iostream>

#include <soemdsp/runtime/control/EvaluateControlGraph.hpp>

using namespace soemdsp::runtime;

namespace
{

void printRow(float input)
{
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
              << "\n";
}

} // namespace

int main()
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "input | linear | ease_in | ease_out | smoothstep\n";
    std::cout << "------+--------+---------+----------+-----------\n";

    printRow(0.0f);
    printRow(0.25f);
    printRow(0.5f);
    printRow(0.75f);
    printRow(1.0f);

    return 0;
}
