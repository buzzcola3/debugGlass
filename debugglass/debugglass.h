#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include "debugglass/subwindow_registry.h"
namespace debugglass {

struct DebugGlassOptions {
    int width = 640;
    int height = 480;
    std::string title = "DebugGlass";
    std::chrono::milliseconds frame_time{16};
};

class DebugGlass {
public:
    DebugGlass() = default;
    ~DebugGlass();

    DebugGlass(const DebugGlass&) = delete;
    DebugGlass& operator=(const DebugGlass&) = delete;

    bool Run(const DebugGlassOptions& options = DebugGlassOptions{});
    void Stop();
    bool IsRunning() const;

    SubWindowRegistry windows;

private:
    void ThreadMain(DebugGlassOptions options);

    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
};

}  // namespace debugglass
