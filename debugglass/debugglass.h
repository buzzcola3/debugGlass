#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
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
    using BackgroundRenderCallback = std::function<void()>;
    void SetBackgroundRenderer(BackgroundRenderCallback callback);

    SubWindowRegistry windows;

private:
    void ThreadMain(DebugGlassOptions options);

    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    mutable std::mutex background_mutex_;
    BackgroundRenderCallback background_callback_;
};

}  // namespace debugglass
