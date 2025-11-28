#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "debugglass/widgets/graph.h"
#include "debugglass/widgets/structure.h"
#include "debugglass/widgets/variable.h"
#include "debugglass/widgets/window_content.h"

namespace debugglass {

class Tab {
public:
    using RenderCallback = std::function<void()>;

    explicit Tab(std::string label);

    const std::string& label() const noexcept { return label_; }

    void SetRenderCallback(RenderCallback callback);

    Graph& AddGraph(std::string label);
    Variable& AddVariable(std::string label);
    Structure& AddStructure(std::string label);

    void Render() const;

private:
    std::string label_;
    mutable std::mutex content_mutex_;
    RenderCallback callback_;
    std::vector<std::shared_ptr<WindowContent>> widgets_;
};

}  // namespace debugglass
