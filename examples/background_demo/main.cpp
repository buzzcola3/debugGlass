#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include <glad/glad.h>
#include <imgui.h>

#include "debugglass/debugglass.h"

namespace {
constexpr auto kDemoDuration = std::chrono::seconds(15);
}

int main() {
    debugglass::DebugGlass monitor;

    const auto start_time = std::chrono::steady_clock::now();
    monitor.SetBackgroundRenderer([start_time]() {
        const auto now = std::chrono::steady_clock::now();
        const float t = std::chrono::duration_cast<std::chrono::duration<float>>(now - start_time).count();
        const float red = 0.2f + 0.3f * (0.5f + 0.5f * std::sin(t * 0.7f));
        const float green = 0.2f + 0.3f * (0.5f + 0.5f * std::sin(t * 0.9f + 1.0f));
        const float blue = 0.3f + 0.4f * (0.5f + 0.5f * std::sin(t * 1.1f + 2.0f));
        glClearColor(red, green, blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    });

    auto& info_window = monitor.windows.add("Overlay Info");
    auto& info_tab = info_window.tabs.add("main");
    info_tab.SetRenderCallback([]() {
        ImGui::TextUnformatted("Background driven by SetBackgroundRenderer()");
        ImGui::Spacing();
        ImGui::TextUnformatted("Drag this window around to see the animated backdrop.");
    });

    debugglass::DebugGlassOptions options;
    options.title = "DebugGlass Background Demo";

    if (!monitor.Run(options)) {
        std::cerr << "Failed to start DebugGlass" << std::endl;
        return 1;
    }

    std::cout << "Background demo running" << std::endl;
    std::this_thread::sleep_for(kDemoDuration);

    monitor.Stop();
    return 0;
}
