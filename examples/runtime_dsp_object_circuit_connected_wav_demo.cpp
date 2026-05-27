#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "WriteMono16Wav.hpp"

#include <soemdsp/runtime/debug/PrintGraph.hpp>
#include <soemdsp/runtime/dsp/ApplyDspBinding.hpp>
#include <soemdsp/runtime/dsp/DspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/PrintDspBlockPhaseReport.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBinding.hpp>
#include <soemdsp/runtime/dsp/ValidateDspBindingTargets.hpp>
#include <soemdsp/runtime/dsp/WriteDspBlockPhaseReport.hpp>
#include <soemdsp/runtime/nodes/AudioMultiply.hpp>
#include <soemdsp/runtime/nodes/AudioOutput.hpp>
#include <soemdsp/runtime/nodes/SineOscillator.hpp>
#include <soemdsp/runtime/serialization/WriteConnections.hpp>
#include <soemdsp/soemdsp.hpp>

using namespace soemdsp::runtime;
using namespace soemdsp::runtime::nodes;

namespace
{
constexpr int sampleRate = 44100;
constexpr double seconds = 1.0;
constexpr double twoPi = 6.28318530717958647692;
constexpr const char* demoName = "runtime_dsp_object_circuit_connected_wav_demo";

struct TinyOscillatorDsp
{
    float* frequency{};
    double phase{};

    float processSample() noexcept
    {
        const float hz = frequency != nullptr ? *frequency : 440.0f;
        const float sample = static_cast<float>(std::sin(phase));

        phase += twoPi * static_cast<double>(hz) / sampleRate;
        while (phase >= twoPi)
        {
            phase -= twoPi;
        }

        return sample;
    }
};

struct TinyGainDsp
{
    float* amplitude{};

    float processSample(float input) const noexcept
    {
        const float gain = amplitude != nullptr ? *amplitude : 0.0f;
        return input * gain;
    }
};

struct PhaseAudioMeasurement
{
    double measuredFrequency{};
    double peak{};
    double rms{};
    double min{};
    double max{};
    double dcOffset{};
};

PhaseAudioMeasurement measurePhaseAudio(
  const std::vector<float>& samples,
  std::size_t startFrame,
  std::size_t endFrame)
{
    PhaseAudioMeasurement measurement;
    if (startFrame >= samples.size() || endFrame <= startFrame)
    {
        return measurement;
    }

    endFrame = std::min(endFrame, samples.size());
    double sum = 0.0;
    double squareSum = 0.0;
    measurement.min = std::numeric_limits<double>::infinity();
    measurement.max = -std::numeric_limits<double>::infinity();

    std::vector<double> crossings;
    double previous = samples[startFrame];
    for (std::size_t frame = startFrame; frame < endFrame; ++frame)
    {
        const double sample = samples[frame];
        measurement.min = std::min(measurement.min, sample);
        measurement.max = std::max(measurement.max, sample);
        measurement.peak = std::max(measurement.peak, std::abs(sample));
        sum += sample;
        squareSum += sample * sample;

        if (frame > startFrame && previous < 0.0 && sample >= 0.0)
        {
            const double span = sample - previous;
            const double offset = span == 0.0 ? 0.0 : -previous / span;
            crossings.push_back(static_cast<double>(frame - 1) + offset);
        }
        previous = sample;
    }

    const auto frames = static_cast<double>(endFrame - startFrame);
    measurement.dcOffset = sum / frames;
    measurement.rms = std::sqrt(squareSum / frames);

    if (crossings.size() >= 2)
    {
        const double spanSeconds =
          (crossings.back() - crossings.front()) / sampleRate;
        if (spanSeconds > 0.0)
        {
            measurement.measuredFrequency =
              static_cast<double>(crossings.size() - 1) / spanSeconds;
        }
    }

    return measurement;
}

void addParameter(
  Node& node,
  const char* id,
  const char* name,
  float value,
  float minValue,
  float midValue,
  float maxValue)
{
    Parameter parameter;
    parameter.id = id;
    parameter.name = name;
    parameter.value = value;
    parameter.defaultValue = value;
    parameter.minValue = minValue;
    parameter.midValue = midValue;
    parameter.maxValue = maxValue;
    node.parameters.push_back(parameter);
}

Circuit createCircuit()
{
    Circuit circuit;

    auto oscillator = std::make_unique<SineOscillator>();
    oscillator->id = 1;
    oscillator->name = "Tiny Oscillator";
    oscillator->description = "Circuit node describing caller-owned TinyOscillatorDsp";
    addParameter(*oscillator, "frequency", "Frequency", 220.0f, 20.0f, 440.0f, 2000.0f);

    auto gain = std::make_unique<AudioMultiply>();
    gain->id = 2;
    gain->name = "Tiny Gain";
    gain->description = "Circuit node describing caller-owned TinyGainDsp";
    addParameter(*gain, "amplitude", "Amplitude", 0.2f, 0.0f, 0.5f, 1.0f);

    auto output = std::make_unique<AudioOutput>();
    output->id = 3;
    output->name = "Audio Out";

    circuit.nodes.push_back(std::move(oscillator));
    circuit.nodes.push_back(std::move(gain));
    circuit.nodes.push_back(std::move(output));
    circuit.prepare();

    const bool oscToGain =
      circuit.connect(
        *circuit.nodes[0],
        circuit.nodes[0]->outputs[0],
        *circuit.nodes[1],
        circuit.nodes[1]->inputs[0]);
    const bool gainToOutput =
      circuit.connect(
        *circuit.nodes[1],
        circuit.nodes[1]->outputs[0],
        *circuit.nodes[2],
        circuit.nodes[2]->inputs[0]);

    circuit.output = &circuit.nodes[2]->inputs[0];

    std::cout << "Circuit connection oscillator->gain: "
              << (oscToGain ? "true" : "false")
              << "\n";
    std::cout << "Circuit connection gain->output: "
              << (gainToOutput ? "true" : "false")
              << "\n";

    return circuit;
}

DspObjectBinding createChainBinding(
  float& frequencyMemory,
  float& amplitudeMemory)
{
    DspObjectBinding binding;
    binding.nodeId = 1;
    binding.objectType = "TinyCircuitConnectedDspChain";
    binding.objectName = "Tiny Circuit-Connected DSP Chain";

    binding.parameterBindings.push_back({
      1,
      "frequency",
      DspBindingTargetKind::MemorySlot,
      "oscillator.frequency",
      0,
      &frequencyMemory });

    binding.parameterBindings.push_back({
      2,
      "amplitude",
      DspBindingTargetKind::MemorySlot,
      "gain.amplitude",
      1,
      &amplitudeMemory });

    return binding;
}

void preflightBinding(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit)
{
    const auto structuralValidation = validateDspObjectBinding(binding);
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
    const auto result = applyDspParameterBindings(binding, circuit);

    report.applyOk = report.applyOk && result.ok;
    report.parametersApplied += result.parametersApplied;
    report.applyMessages += result.messages.size();
}

void renderFrames(
  DspBlockPhaseReport& report,
  TinyOscillatorDsp& oscillator,
  TinyGainDsp& gain,
  std::vector<float>& samples,
  std::size_t frameCount)
{
    for (std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const auto oscillatorOut = oscillator.processSample();
        const auto gainOut = gain.processSample(oscillatorOut);
        samples.push_back(gainOut);
        ++report.samplesProcessed;
    }

    report.processOk = true;
}

void runRenderPhase(
  DspBlockPhaseReport& report,
  const DspObjectBinding& binding,
  const Circuit& circuit,
  TinyOscillatorDsp& oscillator,
  TinyGainDsp& gain,
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
        renderFrames(report, oscillator, gain, samples, frameCount);
    }
}

void printAndWriteReport(
  const char* label,
  const DspBlockPhaseReport& report,
  const char* path)
{
    std::cout << label << "\n";
    printDspBlockPhaseReport(report);

    const auto wroteReport =
      writeDspBlockPhaseReportTextFile(report, path);
    std::cout << "phase report file written: "
              << (wroteReport ? "true" : "false")
              << "\n";
}

void printDspBlockPhaseReportBody(
  const DspBlockPhaseReport& report,
  std::ostream& os)
{
    os << "preflight ok: " << (report.preflightOk ? "true" : "false") << "\n"
       << "apply ok: " << (report.applyOk ? "true" : "false") << "\n"
       << "process ok: " << (report.processOk ? "true" : "false") << "\n"
       << "bindings checked: " << report.bindingsChecked << "\n"
       << "preflight messages: " << report.preflightMessages << "\n"
       << "parameters applied: " << report.parametersApplied << "\n"
       << "apply messages: " << report.applyMessages << "\n"
       << "samples processed: " << report.samplesProcessed << "\n";
}

bool writeCombinedRenderReport(
  const char* path,
  const Circuit& circuit,
  const DspBlockPhaseReport& firstReport,
  const DspBlockPhaseReport& secondReport,
  const soemdsp::examples::Mono16WavWriteReport& wavReport,
  bool frequencyChanged,
  bool amplitudeChanged,
  float firstFrequency,
  float firstAmplitude,
  float secondFrequency,
  float secondAmplitude)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    stream << "[CIRCUIT CONNECTED WAV RENDER REPORT]\n"
           << "circuit nodes: " << circuit.nodes.size() << "\n"
           << "circuit connections: " << circuit.connections.size() << "\n"
           << "connection 1: Tiny Oscillator.Out -> Tiny Gain.A\n"
           << "connection 2: Tiny Gain.Out -> Audio Out.In\n"
           << "caller owns DSP objects: true\n"
           << "caller owns processing order: true\n"
           << "frequency setter ok: " << (frequencyChanged ? "true" : "false") << "\n"
           << "amplitude setter ok: " << (amplitudeChanged ? "true" : "false") << "\n"
           << "first half frequency: " << firstFrequency << "\n"
           << "first half amplitude: " << firstAmplitude << "\n"
           << "second half frequency: " << secondFrequency << "\n"
           << "second half amplitude: " << secondAmplitude << "\n\n"
           << "[FIRST PHASE]\n";

    printDspBlockPhaseReportBody(firstReport, stream);
    stream << "\n[SECOND PHASE]\n";
    printDspBlockPhaseReportBody(secondReport, stream);
    stream << "\n";
    soemdsp::examples::printMono16WavWriteReport(wavReport, stream);

    return static_cast<bool>(stream);
}

void writeJsonBool(
  std::ostream& stream,
  int indent,
  const char* key,
  bool value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\"" << key << "\": " << (value ? "true" : "false")
           << (trailingComma ? "," : "") << "\n";
}

void writeJsonNumber(
  std::ostream& stream,
  int indent,
  const char* key,
  std::size_t value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\"" << key << "\": " << value
           << (trailingComma ? "," : "") << "\n";
}

void writeJsonFloat(
  std::ostream& stream,
  int indent,
  const char* key,
  float value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\"" << key << "\": " << value
           << (trailingComma ? "," : "") << "\n";
}

void writeJsonDouble(
  std::ostream& stream,
  int indent,
  const char* key,
  double value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\"" << key << "\": " << value
           << (trailingComma ? "," : "") << "\n";
}

void writeJsonString(
  std::ostream& stream,
  int indent,
  const char* key,
  const char* value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\"" << key << "\": \"" << value << "\""
           << (trailingComma ? "," : "") << "\n";
}

void writeArtifactLinkManifest(
  std::ostream& stream,
  const char* label,
  const char* kind,
  const char* path,
  bool trailingComma,
  const char* phase = nullptr)
{
    stream << "    {\n";
    writeJsonString(stream, 6, "label", label, true);
    writeJsonString(stream, 6, "kind", kind, true);
    writeJsonString(stream, 6, "path", path, phase != nullptr);
    if (phase != nullptr)
    {
        writeJsonString(stream, 6, "phase", phase, false);
    }
    stream << "    }" << (trailingComma ? "," : "") << "\n";
}

void writePhaseManifest(
  std::ostream& stream,
  const char* name,
  const DspBlockPhaseReport& report,
  std::size_t startFrame,
  std::size_t endFrame,
  bool trailingComma)
{
    stream << "    {\n";
    writeJsonString(stream, 6, "name", name, true);
    writeJsonBool(stream, 6, "preflightOk", report.preflightOk, true);
    writeJsonBool(stream, 6, "applyOk", report.applyOk, true);
    writeJsonBool(stream, 6, "processOk", report.processOk, true);
    writeJsonNumber(stream, 6, "bindingsChecked", report.bindingsChecked, true);
    writeJsonNumber(stream, 6, "parametersApplied", report.parametersApplied, true);
    writeJsonNumber(stream, 6, "samplesProcessed", report.samplesProcessed, true);
    writeJsonNumber(stream, 6, "startFrame", startFrame, true);
    writeJsonNumber(stream, 6, "endFrame", endFrame, false);
    stream << "    }" << (trailingComma ? "," : "") << "\n";
}

void writeParameterResyncManifest(
  std::ostream& stream,
  const char* parameterId,
  bool changed,
  float firstValue,
  float secondValue,
  bool trailingComma)
{
    stream << "    \"" << parameterId << "\": {\n";
    writeJsonBool(stream, 6, "changed", changed, true);
    writeJsonFloat(stream, 6, "first", firstValue, true);
    writeJsonFloat(stream, 6, "second", secondValue, false);
    stream << "    }" << (trailingComma ? "," : "") << "\n";
}

void writePhaseAudioMeasurementManifest(
  std::ostream& stream,
  const char* name,
  const PhaseAudioMeasurement& measurement,
  bool trailingComma)
{
    stream << "    {\n";
    writeJsonString(stream, 6, "name", name, true);
    writeJsonDouble(stream, 6, "measuredFrequency", measurement.measuredFrequency, true);
    writeJsonDouble(stream, 6, "peak", measurement.peak, true);
    writeJsonDouble(stream, 6, "rms", measurement.rms, true);
    writeJsonDouble(stream, 6, "min", measurement.min, true);
    writeJsonDouble(stream, 6, "max", measurement.max, true);
    writeJsonDouble(stream, 6, "dcOffset", measurement.dcOffset, false);
    stream << "    }" << (trailingComma ? "," : "") << "\n";
}

bool writeHtmlAudioReport(
  const char* path,
  const soemdsp::examples::Mono16WavWriteReport& wavReport,
  float firstFrequency,
  float firstAmplitude,
  float secondFrequency,
  float secondAmplitude)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    stream << "<!doctype html>\n"
           << "<html lang=\"en\">\n"
           << "<head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           << "<title>Circuit Connected WAV Demo</title>\n"
           << "<style>body{margin:0;font-family:Arial,sans-serif;background:#111;color:#f1f1f1;line-height:1.45;}main{max-width:880px;margin:0 auto;padding:32px;}audio{width:100%;margin:16px 0 24px;}section{border-top:1px solid #333;padding:18px 0;}dl{display:grid;grid-template-columns:minmax(180px,260px)1fr;gap:8px 18px;}dt{color:#aaa;}dd{margin:0;font-family:Consolas,monospace;}a{color:#8ed2ff;}</style>\n"
           << "</head><body><main>\n"
           << "<h1>Circuit Connected WAV Demo</h1>\n"
           << "<audio controls preload=\"metadata\" src=\"" << wavReport.path << "\"></audio>\n"
           << "<section><h2>C++ Connections</h2><dl>\n"
           << "<dt>connection 1</dt><dd>Tiny Oscillator.Out -&gt; Tiny Gain.A</dd>\n"
           << "<dt>connection 2</dt><dd>Tiny Gain.Out -&gt; Audio Out.In</dd>\n"
           << "<dt>caller owns DSP objects</dt><dd>true</dd>\n"
           << "<dt>caller owns processing order</dt><dd>true</dd>\n"
           << "</dl></section>\n"
           << "<section><h2>Render</h2><dl>\n"
           << "<dt>first half frequency</dt><dd>" << firstFrequency << "</dd>\n"
           << "<dt>first half amplitude</dt><dd>" << firstAmplitude << "</dd>\n"
           << "<dt>second half frequency</dt><dd>" << secondFrequency << "</dd>\n"
           << "<dt>second half amplitude</dt><dd>" << secondAmplitude << "</dd>\n"
           << "</dl></section>\n"
           << "<section><h2>Artifact Links</h2><ul>\n"
           << "<li><a href=\"" << wavReport.path << "\">WAV artifact</a></li>\n"
           << "<li><a href=\"" << demoName << ".manifest.json\">Artifact manifest</a></li>\n"
           << "<li><a href=\"" << demoName << ".summary.txt\">Combined text summary</a></li>\n"
           << "<li><a href=\"" << demoName << ".wav.txt\">WAV metadata report</a></li>\n"
           << "<li><a href=\"" << demoName << ".first.txt\">First phase report</a></li>\n"
           << "<li><a href=\"" << demoName << ".second.txt\">Second phase report</a></li>\n"
           << "</ul></section>\n"
           << "</main></body></html>\n";

    return static_cast<bool>(stream);
}

bool writeArtifactManifest(
  const char* path,
  const Circuit& circuit,
  const DspBlockPhaseReport& firstReport,
  const DspBlockPhaseReport& secondReport,
  const soemdsp::examples::Mono16WavWriteReport& wavReport,
  const PhaseAudioMeasurement& firstMeasurement,
  const PhaseAudioMeasurement& secondMeasurement,
  bool frequencyChanged,
  bool amplitudeChanged,
  float firstFrequency,
  float firstAmplitude,
  float secondFrequency,
  float secondAmplitude,
  bool wroteWavReport,
  bool wroteCombinedReport,
  bool wroteHtmlReport,
  bool wroteConnectionReport)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    const bool allOk =
      circuit.connections.size() == 2 &&
      frequencyChanged &&
      amplitudeChanged &&
      firstReport.preflightOk &&
      firstReport.applyOk &&
      firstReport.processOk &&
      secondReport.preflightOk &&
      secondReport.applyOk &&
      secondReport.processOk &&
      wavReport.wrote &&
      wroteWavReport &&
      wroteCombinedReport &&
      wroteHtmlReport &&
      wroteConnectionReport;

    stream << "{\n"
           << "  \"demo\": \"" << demoName << "\",\n"
           << "  \"kind\": \"demo-local-circuit-connected-wav-artifacts\",\n"
           << "  \"runtimeApi\": false,\n"
           << "  \"scheduler\": false,\n"
           << "  \"audioEngine\": false,\n"
           << "  \"allOk\": " << (allOk ? "true" : "false") << ",\n"
           << "  \"parameterSetters\": {\n"
           << "    \"frequency\": " << (frequencyChanged ? "true" : "false") << ",\n"
           << "    \"amplitude\": " << (amplitudeChanged ? "true" : "false") << "\n"
           << "  },\n"
           << "  \"circuitConnections\": {\n"
           << "    \"count\": " << circuit.connections.size() << ",\n"
           << "    \"describesProcessingChain\": true,\n"
           << "    \"callerOwnsDspObjects\": true,\n"
           << "    \"callerOwnsProcessingOrder\": true\n"
           << "  },\n"
           << "  \"parameterResync\": {\n";

    writeParameterResyncManifest(stream, "frequency", frequencyChanged, firstFrequency, secondFrequency, true);
    writeParameterResyncManifest(stream, "amplitude", amplitudeChanged, firstAmplitude, secondAmplitude, false);

    const auto firstStartFrame = static_cast<std::size_t>(0);
    const auto firstEndFrame = firstReport.samplesProcessed;
    const auto secondStartFrame = firstEndFrame;
    const auto secondEndFrame = firstEndFrame + secondReport.samplesProcessed;

    stream << "  },\n"
           << "  \"phases\": [\n";
    writePhaseManifest(stream, "first", firstReport, firstStartFrame, firstEndFrame, true);
    writePhaseManifest(stream, "second", secondReport, secondStartFrame, secondEndFrame, false);

    stream << "  ],\n"
           << "  \"phaseAudioMeasurements\": [\n";
    writePhaseAudioMeasurementManifest(stream, "first", firstMeasurement, true);
    writePhaseAudioMeasurementManifest(stream, "second", secondMeasurement, false);

    stream << "  ],\n"
           << "  \"wav\": {\n"
           << "    \"path\": \"" << wavReport.path << "\",\n"
           << "    \"wrote\": " << (wavReport.wrote ? "true" : "false") << ",\n"
           << "    \"sampleRate\": " << wavReport.sampleRate << ",\n"
           << "    \"channels\": " << wavReport.channelCount << ",\n"
           << "    \"bitDepth\": " << wavReport.bitsPerSample << ",\n"
           << "    \"frames\": " << wavReport.frames << ",\n"
           << "    \"dataBytes\": " << wavReport.dataBytes << ",\n"
           << "    \"fileBytes\": " << wavReport.fileBytes << "\n"
           << "  },\n"
           << "  \"artifacts\": {\n"
           << "    \"connectionReport\": \"" << demoName << ".connections.txt\",\n"
           << "    \"wavReport\": \"" << demoName << ".wav.txt\",\n"
           << "    \"firstPhaseReport\": \"" << demoName << ".first.txt\",\n"
           << "    \"secondPhaseReport\": \"" << demoName << ".second.txt\",\n"
           << "    \"textSummary\": \"" << demoName << ".summary.txt\",\n"
           << "    \"htmlReport\": \"" << demoName << ".html\",\n"
           << "    \"manifest\": \"" << demoName << ".manifest.json\"\n"
           << "  },\n"
           << "  \"artifactLinks\": [\n";

    writeArtifactLinkManifest(stream, "HTML report", "entry-point", "runtime_dsp_object_circuit_connected_wav_demo.html", true);
    writeArtifactLinkManifest(stream, "WAV artifact", "audio", wavReport.path.c_str(), true);
    writeArtifactLinkManifest(stream, "Artifact manifest", "manifest", "runtime_dsp_object_circuit_connected_wav_demo.manifest.json", true);
    writeArtifactLinkManifest(stream, "Combined text summary", "text-summary", "runtime_dsp_object_circuit_connected_wav_demo.summary.txt", true);
    writeArtifactLinkManifest(stream, "WAV metadata report", "wav-report", "runtime_dsp_object_circuit_connected_wav_demo.wav.txt", true);
    writeArtifactLinkManifest(stream, "Circuit connection report", "connection-report", "runtime_dsp_object_circuit_connected_wav_demo.connections.txt", true);
    writeArtifactLinkManifest(stream, "First phase report", "phase-report", "runtime_dsp_object_circuit_connected_wav_demo.first.txt", true, "first");
    writeArtifactLinkManifest(stream, "Second phase report", "phase-report", "runtime_dsp_object_circuit_connected_wav_demo.second.txt", false, "second");

    stream << "  ],\n"
           << "  \"sandboxHandoff\": {\n"
           << "    \"contract\": \"soemdsp-demo-local-sandbox-handoff\",\n"
           << "    \"contractVersion\": 1,\n"
           << "    \"entryPoint\": \"runtime_dsp_object_circuit_connected_wav_demo.html\",\n"
           << "    \"primaryAudioArtifact\": \"runtime_dsp_object_circuit_connected_wav_demo.wav\",\n"
           << "    \"inspectionMode\": \"mouse-and-ears\",\n"
           << "    \"callerOwnsProcessingOrder\": true,\n"
           << "    \"callerOwnsDspObjects\": true,\n"
           << "    \"circuitOwnsDspObjects\": false,\n"
           << "    \"dspObjectsKnowCircuit\": false,\n"
           << "    \"serializesPatch\": false,\n"
           << "    \"ownsAudioEngine\": false,\n"
           << "    \"ownsScheduler\": false\n"
           << "  }\n"
           << "}\n";

    return static_cast<bool>(stream);
}
} // namespace

int main()
{
    auto circuit = createCircuit();

    float frequencyMemory = 0.0f;
    float amplitudeMemory = 0.0f;
    TinyOscillatorDsp oscillator{ &frequencyMemory };
    TinyGainDsp gain{ &amplitudeMemory };

    const auto binding = createChainBinding(frequencyMemory, amplitudeMemory);
    const auto totalFrames = static_cast<std::size_t>(sampleRate * seconds);
    const auto halfFrames = totalFrames / 2;

    std::vector<float> samples;
    samples.reserve(totalFrames);

    std::cout << "[DSP OBJECT CIRCUIT CONNECTED WAV]\n";
    std::cout << "[CIRCUIT CONNECTIONS]\n";
    soemdsp::runtime::debug::printConnections(circuit);

    DspBlockPhaseReport firstReport;
    runRenderPhase(firstReport, binding, circuit, oscillator, gain, samples, halfFrames);
    printAndWriteReport(
      "[FIRST RENDER PHASE REPORT]",
      firstReport,
      "runtime_dsp_object_circuit_connected_wav_demo.first.txt");
    if (!firstReport.preflightOk || !firstReport.applyOk || !firstReport.processOk)
    {
        std::cerr << "First render phase failed; skipping render.\n";
        return 1;
    }

    const float firstFrequency = frequencyMemory;
    const float firstAmplitude = amplitudeMemory;

    const auto frequencyChanged = circuit.setParameterValue(1, "frequency", 440.0f);
    const auto amplitudeChanged = circuit.setParameterValue(2, "amplitude", 0.35f);

    DspBlockPhaseReport secondReport;
    runRenderPhase(
      secondReport,
      binding,
      circuit,
      oscillator,
      gain,
      samples,
      totalFrames - samples.size());
    printAndWriteReport(
      "[SECOND RENDER PHASE REPORT]",
      secondReport,
      "runtime_dsp_object_circuit_connected_wav_demo.second.txt");
    if (!secondReport.preflightOk || !secondReport.applyOk || !secondReport.processOk)
    {
        std::cerr << "Second render phase failed; skipping render.\n";
        return 1;
    }

    const float secondFrequency = frequencyMemory;
    const float secondAmplitude = amplitudeMemory;
    const auto firstMeasurement =
      measurePhaseAudio(samples, 0, firstReport.samplesProcessed);
    const auto secondMeasurement =
      measurePhaseAudio(
        samples,
        firstReport.samplesProcessed,
        firstReport.samplesProcessed + secondReport.samplesProcessed);

    const std::string path = "runtime_dsp_object_circuit_connected_wav_demo.wav";
    const auto wavReport =
      soemdsp::examples::writeMono16WavWithReport(path, samples, sampleRate);
    soemdsp::examples::printMono16WavWriteReport(wavReport);
    const auto wroteWavReport =
      soemdsp::examples::writeMono16WavWriteReportTextFile(
        wavReport,
        "runtime_dsp_object_circuit_connected_wav_demo.wav.txt");

    if (!wavReport.wrote)
    {
        std::cerr << "Failed to write " << path << "\n";
        return 1;
    }

    const auto wroteConnectionReport =
      writeConnectionsTextFile(
        circuit.snapshot(),
        "runtime_dsp_object_circuit_connected_wav_demo.connections.txt");
    const auto wroteCombinedReport =
      writeCombinedRenderReport(
        "runtime_dsp_object_circuit_connected_wav_demo.summary.txt",
        circuit,
        firstReport,
        secondReport,
        wavReport,
        frequencyChanged,
        amplitudeChanged,
        firstFrequency,
        firstAmplitude,
        secondFrequency,
        secondAmplitude);
    const auto wroteHtmlReport =
      writeHtmlAudioReport(
        "runtime_dsp_object_circuit_connected_wav_demo.html",
        wavReport,
        firstFrequency,
        firstAmplitude,
        secondFrequency,
        secondAmplitude);
    const auto wroteArtifactManifest =
      writeArtifactManifest(
        "runtime_dsp_object_circuit_connected_wav_demo.manifest.json",
        circuit,
        firstReport,
        secondReport,
        wavReport,
        firstMeasurement,
        secondMeasurement,
        frequencyChanged,
        amplitudeChanged,
        firstFrequency,
        firstAmplitude,
        secondFrequency,
        secondAmplitude,
        wroteWavReport,
        wroteCombinedReport,
        wroteHtmlReport,
        wroteConnectionReport);

    std::cout << "connection report file written: "
              << (wroteConnectionReport ? "true" : "false") << "\n";
    std::cout << "combined render report file written: "
              << (wroteCombinedReport ? "true" : "false") << "\n";
    std::cout << "html audio report file written: "
              << (wroteHtmlReport ? "true" : "false") << "\n";
    std::cout << "artifact manifest file written: "
              << (wroteArtifactManifest ? "true" : "false") << "\n";
    std::cout << "first half frequency: " << firstFrequency << "\n";
    std::cout << "first half amplitude: " << firstAmplitude << "\n";
    std::cout << "second half frequency: " << secondFrequency << "\n";
    std::cout << "second half amplitude: " << secondAmplitude << "\n";
    std::cout << "source: Circuit connections describe TinyOscillatorDsp -> TinyGainDsp -> audio output\n";
    std::cout << "boundary: Circuit owns graph description only; caller owns DSP objects and processing order\n";

    return wroteArtifactManifest ? 0 : 1;
}
