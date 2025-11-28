#include "debugglass/widgets/tab.h"

#include <imgui.h>

#include <utility>

#include "debugglass/widgets/graph.h"
#include "debugglass/widgets/structure.h"
#include "debugglass/widgets/variable.h"

namespace debugglass {

Tab::Tab(std::string label) : label_(std::move(label)) {}

void Tab::SetRenderCallback(RenderCallback callback) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    callback_ = std::move(callback);
}

Graph& Tab::AddGraph(std::string label) {
    auto graph = std::make_shared<Graph>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(graph);
    return *graph;
}

Variable& Tab::AddVariable(std::string label) {
    auto variable = std::make_shared<Variable>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(variable);
    return *variable;
}

Structure& Tab::AddStructure(std::string label) {
    auto structure = std::make_shared<Structure>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(structure);
    return *structure;
}

void Tab::Render() const {
    Tab::RenderCallback callback_copy;
    std::vector<std::shared_ptr<WindowContent>> widgets_snapshot;
    {
        std::lock_guard<std::mutex> lock(content_mutex_);
        callback_copy = callback_;
        widgets_snapshot = widgets_;
    }

    if (callback_copy) {
        callback_copy();
    }

    for (const auto& widget : widgets_snapshot) {
        if (widget) {
            widget->Render();
        }
    }

    if (!callback_copy && widgets_snapshot.empty()) {
        ImGui::TextUnformatted("No content assigned");
    }
}

}  // namespace debugglass
