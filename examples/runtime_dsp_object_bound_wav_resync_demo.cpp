#include <cmath>
#include <fstream>
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

void printDspBlockPhaseReportBody(
  const DspBlockPhaseReport& report,
  std::ostream& os)
{
    os << "preflight ok: "
       << (report.preflightOk ? "true" : "false")
       << "\n"
       << "apply ok: "
       << (report.applyOk ? "true" : "false")
       << "\n"
       << "process ok: "
       << (report.processOk ? "true" : "false")
       << "\n"
       << "bindings checked: "
       << report.bindingsChecked
       << "\n"
       << "preflight messages: "
       << report.preflightMessages
       << "\n"
       << "parameters applied: "
       << report.parametersApplied
       << "\n"
       << "apply messages: "
       << report.applyMessages
       << "\n"
       << "samples processed: "
       << report.samplesProcessed
       << "\n";
}

bool writeCombinedRenderReport(
  const char* path,
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

    stream << "[BOUND WAV RESYNC RENDER REPORT]\n"
           << "frequency setter ok: "
           << (frequencyChanged ? "true" : "false")
           << "\n"
           << "amplitude setter ok: "
           << (amplitudeChanged ? "true" : "false")
           << "\n"
           << "first half frequency: "
           << firstFrequency
           << "\n"
           << "first half amplitude: "
           << firstAmplitude
           << "\n"
           << "second half frequency: "
           << secondFrequency
           << "\n"
           << "second half amplitude: "
           << secondAmplitude
           << "\n\n"
           << "[FIRST PHASE]\n";

    printDspBlockPhaseReportBody(firstReport, stream);

    stream << "\n[SECOND PHASE]\n";
    printDspBlockPhaseReportBody(secondReport, stream);

    stream << "\n";
    soemdsp::examples::printMono16WavWriteReport(wavReport, stream);

    return static_cast<bool>(stream);
}

void writePhaseHtml(
  std::ofstream& stream,
  const char* heading,
  const DspBlockPhaseReport& report)
{
    stream << "<section>\n"
           << "<h2>"
           << heading
           << "</h2>\n"
           << "<dl>\n"
           << "<dt>preflight ok</dt><dd>"
           << (report.preflightOk ? "true" : "false")
           << "</dd>\n"
           << "<dt>apply ok</dt><dd>"
           << (report.applyOk ? "true" : "false")
           << "</dd>\n"
           << "<dt>process ok</dt><dd>"
           << (report.processOk ? "true" : "false")
           << "</dd>\n"
           << "<dt>bindings checked</dt><dd>"
           << report.bindingsChecked
           << "</dd>\n"
           << "<dt>parameters applied</dt><dd>"
           << report.parametersApplied
           << "</dd>\n"
           << "<dt>samples processed</dt><dd>"
           << report.samplesProcessed
           << "</dd>\n"
           << "</dl>\n"
           << "</section>\n";
}

bool writeHtmlAudioReport(
  const char* path,
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

    stream << "<!doctype html>\n"
           << "<html lang=\"en\">\n"
           << "<head>\n"
           << "<meta charset=\"utf-8\">\n"
           << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           << "<title>Bound WAV Resync Demo</title>\n"
           << "<style>\n"
           << "body{margin:0;font-family:Arial,sans-serif;background:#111;color:#f1f1f1;line-height:1.45;}\n"
           << "main{max-width:880px;margin:0 auto;padding:32px;}\n"
           << "h1,h2{font-weight:600;}\n"
           << "audio{width:100%;margin:16px 0 24px;}\n"
           << "section{border-top:1px solid #333;padding:18px 0;}\n"
           << "dl{display:grid;grid-template-columns:minmax(160px,240px)1fr;gap:8px 18px;}\n"
           << "dt{color:#aaa;}dd{margin:0;font-family:Consolas,monospace;}\n"
           << "ul{padding-left:22px;}li{margin:6px 0;}a{color:#8ed2ff;}\n"
           << "</style>\n"
           << "</head>\n"
           << "<body>\n"
           << "<main>\n"
           << "<h1>Bound WAV Resync Demo</h1>\n"
           << "<audio controls preload=\"metadata\" src=\""
           << wavReport.path
           << "\"></audio>\n"
           << "<section>\n"
           << "<h2>Render</h2>\n"
           << "<dl>\n"
           << "<dt>frequency setter ok</dt><dd>"
           << (frequencyChanged ? "true" : "false")
           << "</dd>\n"
           << "<dt>amplitude setter ok</dt><dd>"
           << (amplitudeChanged ? "true" : "false")
           << "</dd>\n"
           << "<dt>first half frequency</dt><dd>"
           << firstFrequency
           << "</dd>\n"
           << "<dt>first half amplitude</dt><dd>"
           << firstAmplitude
           << "</dd>\n"
           << "<dt>second half frequency</dt><dd>"
           << secondFrequency
           << "</dd>\n"
           << "<dt>second half amplitude</dt><dd>"
           << secondAmplitude
           << "</dd>\n"
           << "</dl>\n"
           << "</section>\n";

    writePhaseHtml(stream, "First Phase", firstReport);
    writePhaseHtml(stream, "Second Phase", secondReport);

    stream << "<section>\n"
           << "<h2>WAV Artifact</h2>\n"
           << "<dl>\n"
           << "<dt>wrote</dt><dd>"
           << (wavReport.wrote ? "true" : "false")
           << "</dd>\n"
           << "<dt>path</dt><dd>"
           << wavReport.path
           << "</dd>\n"
           << "<dt>sample rate</dt><dd>"
           << wavReport.sampleRate
           << "</dd>\n"
           << "<dt>channels</dt><dd>"
           << wavReport.channelCount
           << "</dd>\n"
           << "<dt>bit depth</dt><dd>"
           << wavReport.bitsPerSample
           << "</dd>\n"
           << "<dt>frames</dt><dd>"
           << wavReport.frames
           << "</dd>\n"
           << "<dt>file bytes</dt><dd>"
           << wavReport.fileBytes
           << "</dd>\n"
           << "</dl>\n"
           << "</section>\n"
           << "<section>\n"
           << "<h2>Artifact Links</h2>\n"
           << "<ul>\n"
           << "<li><a href=\""
           << wavReport.path
           << "\">WAV artifact</a></li>\n"
           << "<li><a href=\"runtime_dsp_object_bound_wav_resync_demo.manifest.json\">Artifact manifest</a></li>\n"
           << "<li><a href=\"runtime_dsp_object_bound_wav_resync_demo.summary.txt\">Combined text summary</a></li>\n"
           << "<li><a href=\"runtime_dsp_object_bound_wav_resync_demo.wav.txt\">WAV metadata report</a></li>\n"
           << "<li><a href=\"runtime_dsp_object_bound_wav_resync_demo.first.txt\">First phase report</a></li>\n"
           << "<li><a href=\"runtime_dsp_object_bound_wav_resync_demo.second.txt\">Second phase report</a></li>\n"
           << "</ul>\n"
           << "</section>\n"
           << "</main>\n"
           << "</body>\n"
           << "</html>\n";

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
           << "\""
           << key
           << "\": "
           << (value ? "true" : "false")
           << (trailingComma ? "," : "")
           << "\n";
}

void writeJsonNumber(
  std::ostream& stream,
  int indent,
  const char* key,
  std::size_t value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\""
           << key
           << "\": "
           << value
           << (trailingComma ? "," : "")
           << "\n";
}

void writeJsonString(
  std::ostream& stream,
  int indent,
  const char* key,
  const char* value,
  bool trailingComma)
{
    stream << std::string(static_cast<std::size_t>(indent), ' ')
           << "\""
           << key
           << "\": \""
           << value
           << "\""
           << (trailingComma ? "," : "")
           << "\n";
}

void writeArtifactLinkManifest(
  std::ostream& stream,
  const char* label,
  const char* kind,
  const char* path,
  bool trailingComma)
{
    stream << "    {\n";
    writeJsonString(stream, 6, "label", label, true);
    writeJsonString(stream, 6, "kind", kind, true);
    writeJsonString(stream, 6, "path", path, false);
    stream << "    }"
           << (trailingComma ? "," : "")
           << "\n";
}

void writePhaseManifest(
  std::ostream& stream,
  const char* name,
  const DspBlockPhaseReport& report,
  bool trailingComma)
{
    stream << "    {\n";
    writeJsonString(stream, 6, "name", name, true);
    writeJsonBool(stream, 6, "preflightOk", report.preflightOk, true);
    writeJsonBool(stream, 6, "applyOk", report.applyOk, true);
    writeJsonBool(stream, 6, "processOk", report.processOk, true);
    writeJsonNumber(stream, 6, "bindingsChecked", report.bindingsChecked, true);
    writeJsonNumber(stream, 6, "parametersApplied", report.parametersApplied, true);
    writeJsonNumber(stream, 6, "samplesProcessed", report.samplesProcessed, false);
    stream << "    }"
           << (trailingComma ? "," : "")
           << "\n";
}

bool writeArtifactManifest(
  const char* path,
  const DspBlockPhaseReport& firstReport,
  const DspBlockPhaseReport& secondReport,
  const soemdsp::examples::Mono16WavWriteReport& wavReport,
  bool frequencyChanged,
  bool amplitudeChanged,
  bool wroteWavReport,
  bool wroteCombinedReport,
  bool wroteHtmlReport)
{
    std::ofstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }

    const bool allOk =
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
      wroteHtmlReport;

    stream << "{\n"
           << "  \"demo\": \"runtime_dsp_object_bound_wav_resync_demo\",\n"
           << "  \"kind\": \"demo-local-bound-wav-resync-artifacts\",\n"
           << "  \"runtimeApi\": false,\n"
           << "  \"scheduler\": false,\n"
           << "  \"audioEngine\": false,\n"
           << "  \"allOk\": "
           << (allOk ? "true" : "false")
           << ",\n"
           << "  \"parameterSetters\": {\n"
           << "    \"frequency\": "
           << (frequencyChanged ? "true" : "false")
           << ",\n"
           << "    \"amplitude\": "
           << (amplitudeChanged ? "true" : "false")
           << "\n"
           << "  },\n"
           << "  \"phases\": [\n";

    writePhaseManifest(stream, "first", firstReport, true);
    writePhaseManifest(stream, "second", secondReport, false);

    stream << "  ],\n"
           << "  \"wav\": {\n"
           << "    \"path\": \""
           << wavReport.path
           << "\",\n"
           << "    \"wrote\": "
           << (wavReport.wrote ? "true" : "false")
           << ",\n"
           << "    \"sampleRate\": "
           << wavReport.sampleRate
           << ",\n"
           << "    \"frames\": "
           << wavReport.frames
           << ",\n"
           << "    \"fileBytes\": "
           << wavReport.fileBytes
           << "\n"
           << "  },\n"
           << "  \"artifacts\": {\n"
           << "    \"wavReport\": \"runtime_dsp_object_bound_wav_resync_demo.wav.txt\",\n"
           << "    \"firstPhaseReport\": \"runtime_dsp_object_bound_wav_resync_demo.first.txt\",\n"
           << "    \"secondPhaseReport\": \"runtime_dsp_object_bound_wav_resync_demo.second.txt\",\n"
           << "    \"textSummary\": \"runtime_dsp_object_bound_wav_resync_demo.summary.txt\",\n"
           << "    \"htmlReport\": \"runtime_dsp_object_bound_wav_resync_demo.html\",\n"
           << "    \"manifest\": \"runtime_dsp_object_bound_wav_resync_demo.manifest.json\"\n"
           << "  },\n"
           << "  \"artifactLinks\": [\n";

    writeArtifactLinkManifest(
      stream,
      "HTML report",
      "entry-point",
      "runtime_dsp_object_bound_wav_resync_demo.html",
      true);
    writeArtifactLinkManifest(
      stream,
      "WAV artifact",
      "audio",
      wavReport.path.c_str(),
      true);
    writeArtifactLinkManifest(
      stream,
      "Artifact manifest",
      "manifest",
      "runtime_dsp_object_bound_wav_resync_demo.manifest.json",
      true);
    writeArtifactLinkManifest(
      stream,
      "Combined text summary",
      "text-summary",
      "runtime_dsp_object_bound_wav_resync_demo.summary.txt",
      true);
    writeArtifactLinkManifest(
      stream,
      "WAV metadata report",
      "wav-report",
      "runtime_dsp_object_bound_wav_resync_demo.wav.txt",
      true);
    writeArtifactLinkManifest(
      stream,
      "First phase report",
      "phase-report",
      "runtime_dsp_object_bound_wav_resync_demo.first.txt",
      true);
    writeArtifactLinkManifest(
      stream,
      "Second phase report",
      "phase-report",
      "runtime_dsp_object_bound_wav_resync_demo.second.txt",
      false);

    stream << "  ],\n"
           << "  \"sandboxHandoff\": {\n"
           << "    \"contract\": \"soemdsp-demo-local-sandbox-handoff\",\n"
           << "    \"contractVersion\": 1,\n"
           << "    \"entryPoint\": \"runtime_dsp_object_bound_wav_resync_demo.html\",\n"
           << "    \"primaryAudioArtifact\": \"runtime_dsp_object_bound_wav_resync_demo.wav\",\n"
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

    const auto wroteCombinedReport =
      writeCombinedRenderReport(
        "runtime_dsp_object_bound_wav_resync_demo.summary.txt",
        firstReport,
        secondReport,
        wavReport,
        frequencyChanged,
        amplitudeChanged,
        firstFrequency,
        firstAmplitude,
        secondFrequency,
        secondAmplitude);
    std::cout << "combined render report file written: "
              << (wroteCombinedReport ? "true" : "false")
              << "\n";

    const auto wroteHtmlReport =
      writeHtmlAudioReport(
        "runtime_dsp_object_bound_wav_resync_demo.html",
        firstReport,
        secondReport,
        wavReport,
        frequencyChanged,
        amplitudeChanged,
        firstFrequency,
        firstAmplitude,
        secondFrequency,
        secondAmplitude);
    std::cout << "html audio report file written: "
              << (wroteHtmlReport ? "true" : "false")
              << "\n";

    const auto wroteArtifactManifest =
      writeArtifactManifest(
        "runtime_dsp_object_bound_wav_resync_demo.manifest.json",
        firstReport,
        secondReport,
        wavReport,
        frequencyChanged,
        amplitudeChanged,
        wroteWavReport,
        wroteCombinedReport,
        wroteHtmlReport);
    std::cout << "artifact manifest file written: "
              << (wroteArtifactManifest ? "true" : "false")
              << "\n";

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
