#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include <imgui.h>

#include "debugglass/debugglass.h"

namespace {
constexpr auto kDemoDuration = std::chrono::seconds(50);
}

int main() {
    debugglass::DebugGlass monitor;

    auto& stats_window = monitor.windows.add("Stats");
    stats_window.SetRenderCallback([]() {
        ImGui::TextUnformatted("Live Waveform");
    });
    auto& waveform = stats_window.AddGraph("Waveform");
    waveform.SetRange(0.0f, 1.0f);

    auto& variables_window = monitor.windows.add("Variables");
    variables_window.SetRenderCallback([]() {
        ImGui::TextUnformatted("Tracked Variables");
    });
    auto& systems_structure = variables_window.AddStructure("Systems");
    auto& mode_variable = systems_structure.AddVariable("Mode");
    mode_variable.SetValue("demo");
    auto& fps_variable = systems_structure.AddVariable("FPS Target");
    fps_variable.SetValue(60);
    auto& telemetry_structure = systems_structure.AddStructure("Telemetry");
    auto& latency_variable = telemetry_structure.AddVariable("Latency (ms)");
    latency_variable.SetValue(4.2f);

    auto& logs_structure = systems_structure.AddStructure("Logs");
    auto& latest_event = logs_structure.AddVariable("Latest Event");
    latest_event.SetValue("Initialized renderer");

    debugglass::DebugGlassOptions options;
    options.title = "DebugGlass Subwindow Demo";

    if (!monitor.Run(options)) {
        std::cerr << "Failed to start DebugGlass" << std::endl;
        return 1;
    }

    std::cout << "DebugGlass subwindow demo running" << std::endl;

    auto start = std::chrono::steady_clock::now();
    auto end_time = start + kDemoDuration;
    float phase = 0.0f;
    while (std::chrono::steady_clock::now() < end_time) {
        phase += 0.05f;
        const float next_sample = 0.5f + 0.5f * std::sin(phase);
        waveform.AddValue(next_sample);

        const float latency = 4.0f + 1.0f * std::sin(phase * 0.5f);
        latency_variable.SetValue(latency);
        if (phase < 2.0f) {
            latest_event.SetValue("Connected to telemetry feed");
        } else {
            latest_event.SetValue("Awaiting user commands...");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    monitor.Stop();
    return 0;
}
