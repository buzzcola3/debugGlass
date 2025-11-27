#include "monitor/imgui_monitor.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace {
constexpr auto kDemoDuration = std::chrono::seconds(10);
}

int main() {
    ImGuiMonitor monitor;

    MonitorOptions options;
    options.title = "ImGui Monitor Demo";

    if (!monitor.Run(options)) {
        std::cerr << "Failed to start ImGui monitor" << std::endl;
        return 1;
    }

    std::cout << "Hello World" << std::endl;

    std::this_thread::sleep_for(kDemoDuration);

    monitor.Stop();
    return 0;
}
