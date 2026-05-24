#include <iostream>
#include <memory>

#include <soemdsp/runtime/validation/PrintCircuitValidation.hpp>
#include <soemdsp/runtime/validation/WriteCircuitValidation.hpp>
#include <soemdsp/soemdsp.hpp>

namespace
{

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

Parameter makeParameter(
  std::string id,
  std::string name,
  float value,
  float defaultValue,
  float minValue,
  float midValue,
  float maxValue)
{
    Parameter parameter;
    parameter.id           = std::move(id);
    parameter.name         = std::move(name);
    parameter.value        = value;
    parameter.defaultValue = defaultValue;
    parameter.minValue     = minValue;
    parameter.midValue     = midValue;
    parameter.maxValue     = maxValue;
    return parameter;
}

Circuit createInvalidCircuit()
{
    Circuit circuit;

    auto node = std::make_unique<FloatConstant>(0.5f);
    node->parameters.push_back(
      makeParameter("duplicate", "Duplicate A", 0.5f, 0.5f, 0.0f, 0.5f, 1.0f));
    node->parameters.push_back(
      makeParameter("duplicate", "Duplicate B", 0.25f, 0.25f, 0.0f, 0.5f, 1.0f));
    node->parameters.push_back(
      makeParameter("bad_range", "Bad Range", 0.5f, 0.5f, 2.0f, 1.0f, 1.0f));
    node->parameters.push_back(
      makeParameter("bad_mid", "Bad Mid", 0.5f, 0.5f, 0.0f, 2.0f, 1.0f));
    node->parameters.push_back(
      makeParameter("bad_value", "Bad Value", 2.0f, 0.5f, 0.0f, 0.5f, 1.0f));
    node->parameters.push_back(
      makeParameter("bad_default", "Bad Default", 0.5f, -1.0f, 0.0f, 0.5f, 1.0f));

    circuit.nodes.push_back(std::move(node));
    circuit.prepare();

    return circuit;
}

} // namespace

int main()
{
    const auto circuit = createInvalidCircuit();
    const auto report = validateCircuit(circuit);

    printCircuitValidation(report);

    const auto wroteValidation =
      writeCircuitValidationTextFile(
        report,
        "runtime_validation_demo.validation.txt");
    std::cout << "validation file: "
              << (wroteValidation ? "wrote" : "failed")
              << "\n";

    std::cout << "messageCount: "
              << report.messageCount()
              << "\ninfoCount: "
              << report.infoCount()
              << "\nwarningCount: "
              << report.warningCount()
              << "\nerrorCount: "
              << report.errorCount()
              << "\n";

    return 0;
}
