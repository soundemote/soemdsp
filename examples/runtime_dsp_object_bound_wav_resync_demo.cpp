#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "WriteMono16Wav.hpp"

#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/PrintDspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>
#include <soemdsp/runtime/dsp/WriteDspBlockPhaseReport.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{
constexpr int sampleRate = 44100;
constexpr double seconds = 1.0;
constexpr double twoPi = 6.28318530717958647692;

struct TinySineDsp
{
    float* frequency{};
    float* amplitude{};
    double phase{};

    float processSample() noexcept
    {
        const float hz = frequency != nullptr ? *frequency : 440.0f;
        const float amp = amplitude != nullptr ? *amplitude : 0.0f;
        const float sample =
          static_cast<float>(std::sin(phase) * amp);

        phase += twoPi * static_cast<double>(hz) / sampleRate;
        while (phase >= twoPi)
        {
            phase -= twoPi;
        }

        return sample;
    }
};

std::unique_ptr<FloatConstant> createNodeWithParameter(
  NodeId nodeId,
  const char* parameterId,
  const char* name,
  float value,
  float minValue,
  float midValue,
  float maxValue)
{
    auto node = std::make_unique<FloatConstant>(0.0f);
    node->id = nodeId;
    node->name = name;

    Parameter parameter;
    parameter.id = parameterId;
    parameter.name = name;
    parameter.value = value;
    parameter.defaultValue = value;
    parameter.minValue = minValue;
    parameter.midValue = midValue;
    parameter.maxValue = maxValue;
    node->parameters.push_back(parameter);

    return node;
}

Circuit createCircuit()
{
    Circuit circuit;

    circuit.nodes.push_back(
      createNodeWithParameter(
        1,
        "frequency",
        "Frequency",
        220.0f,
        20.0f,
        440.0f,
        2000.0f));

    circuit.nodes.push_back(
      createNodeWithParameter(
        2,
        "amplitude",
        "Amplitude",
        0.2f,
        0.0f,
        0.5f,
        1.0f));

    circuit.prepare();

    return circuit;
}

DspObjectBinding createSineBinding(
  float& frequencyMemory,
  float& amplitudeMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinySineDsp";
    binding.objectName = "Tiny Sine DSP";

    binding.parameterBindings.push_back({
      1,
      "frequency",
      DspBindingTargetKind::MemorySlot,
      "frequency",
      0,
      &frequencyMemory });

    binding.parameterBindings.push_back({
      2,
      "amplitude",
      DspBindingTargetKind::MemorySlot,
      "amplitude",
      1,
      &amplitudeMemory });

    return binding;
}

void renderFrames(
  DspBlockPhaseReport& report,
  TinySineDsp& oscillator,
  std::vector<float>& samples,
  std::size_t frameCount)
{
    for (std::size_t i = 0; i < frameCount; ++i)
    {
        samples.push_back(oscillator.processSample());
        ++report.samplesProcessed;
    }

    report.processOk = true;
}

void preflightBinding(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    const auto structuralValidation =
      validateDspObjectBinding(binding);
    const auto targetValidation =
      validateDspObjectBindingTargets(binding, circuit);

    report.preflightOk =
      report.preflightOk &&
      structuralValidation.ok() &&
      targetValidation.ok();
    ++report.bindingsChecked;
    report.preflightMessages +=
      structuralValidation.messageCount() +
      targetValidation.messageCount();
}

void applyBinding(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    const auto result =
      applyDspParameterBindings(binding, circuit);

    report.applyOk =
      report.applyOk && result.ok;
    report.parametersApplied +=
      result.parametersApplied;
    report.applyMessages +=
      result.messages.size();
}

void runRenderPhase(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit,
  TinySineDsp& oscillator,
  std::vector<float>& samples,
  std::size_t frameCount)
{
    preflightBinding(report, binding, circuit);

    if (report.preflightOk)
    {
        applyBinding(report, binding, circuit);
    }

    if (report.preflightOk && report.applyOk)
    {
        renderFrames(report, oscillator, samples, frameCount);
    }
}

void printAndWriteReport(
  const char* label,
  const DspBlockPhaseReport& report,
  const char* path)
{
    std::cout << label
              << "\n";
    printDspBlockPhaseReport(report);

    const auto wroteReport =
      writeDspBlockPhaseReportTextFile(report, path);
    std::cout << "phase report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";
}
} // namespace

int main()
{
    auto circuit = createCircuit();

    float frequencyMemory = 0.0f;
    float amplitudeMemory = 0.0f;
    TinySineDsp oscillator{ &frequencyMemory, &amplitudeMemory };

    const auto binding =
      createSineBinding(frequencyMemory, amplitudeMemory);
    const auto totalFrames =
      static_cast<std::size_t>(sampleRate * seconds);
    const auto halfFrames = totalFrames / 2;

    std::vector<float> samples;
    samples.reserve(totalFrames);

    std::cout << "[DSP OBJECT BOUND WAV RESYNC]\n";

    DspBlockPhaseReport firstReport;
    runRenderPhase(
      firstReport,
      binding,
      circuit,
      oscillator,
      samples,
      halfFrames);
    printAndWriteReport(
      "[FIRST RENDER PHASE REPORT]",
      firstReport,
      "runtime_dsp_object_bound_wav_resync_demo.first.txt");
    if (!firstReport.preflightOk ||
        !firstReport.applyOk ||
        !firstReport.processOk)
    {
        std::cerr << "First render phase failed; skipping render.\n";
        return 1;
    }

    const float firstFrequency = frequencyMemory;
    const float firstAmplitude = amplitudeMemory;

    const auto frequencyChanged =
      circuit.setParameterValue(1, "frequency", 440.0f);
    const auto amplitudeChanged =
      circuit.setParameterValue(2, "amplitude", 0.35f);

    std::cout << "set frequency returned: "
              << (frequencyChanged ? "true" : "false")
              << "\n";
    std::cout << "set amplitude returned: "
              << (amplitudeChanged ? "true" : "false")
              << "\n";

    DspBlockPhaseReport secondReport;
    runRenderPhase(
      secondReport,
      binding,
      circuit,
      oscillator,
      samples,
      totalFrames - samples.size());
    printAndWriteReport(
      "[SECOND RENDER PHASE REPORT]",
      secondReport,
      "runtime_dsp_object_bound_wav_resync_demo.second.txt");
    if (!secondReport.preflightOk ||
        !secondReport.applyOk ||
        !secondReport.processOk)
    {
        std::cerr << "Second render phase failed; skipping render.\n";
        return 1;
    }

    const float secondFrequency = frequencyMemory;
    const float secondAmplitude = amplitudeMemory;

    const std::string path = "runtime_dsp_object_bound_wav_resync_demo.wav";
    const auto wavReport =
      soemdsp::examples::writeMono16WavWithReport(
        path,
        samples,
        sampleRate);
    soemdsp::examples::printMono16WavWriteReport(wavReport);
    const auto wroteWavReport =
      soemdsp::examples::writeMono16WavWriteReportTextFile(
        wavReport,
        "runtime_dsp_object_bound_wav_resync_demo.wav.txt");
    std::cout << "wav report file written: "
              << (wroteWavReport ? "true" : "false")
              << "\n";

    if (!wavReport.wrote)
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    std::cout << "first half frequency: "
              << firstFrequency
              << "\n";
    std::cout << "first half amplitude: "
              << firstAmplitude
              << "\n";
    std::cout << "second half frequency: "
              << secondFrequency
              << "\n";
    std::cout << "second half amplitude: "
              << secondAmplitude
              << "\n";
    std::cout << "source: Circuit parameter changes -> binding resync -> external memory -> TinySineDsp\n";

    return 0;
}
