#include "debugglass/subwindow_registry.h"

#include <utility>

#include <imgui.h>

#include "debugglass/widgets/graph.h"
#include "debugglass/widgets/structure.h"
#include "debugglass/widgets/variable.h"

namespace debugglass {

SubWindow::SubWindow(std::string name) : name_(std::move(name)) {}

void SubWindow::SetRenderCallback(RenderCallback callback) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    callback_ = std::move(callback);
}

Graph& SubWindow::AddGraph(std::string label) {
    auto graph = std::make_shared<Graph>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(graph);
    return *graph;
}

Graph* SubWindow::FindGraph(const std::string& label) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& widget : widgets_) {
        auto graph = std::dynamic_pointer_cast<Graph>(widget);
        if (graph && graph->label() == label) {
            return graph.get();
        }
    }
    return nullptr;
}

const Graph* SubWindow::FindGraph(const std::string& label) const {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& widget : widgets_) {
        auto graph = std::dynamic_pointer_cast<Graph>(widget);
        if (graph && graph->label() == label) {
            return graph.get();
        }
    }
    return nullptr;
}

Variable& SubWindow::AddVariable(std::string label) {
    auto variable = std::make_shared<Variable>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(variable);
    return *variable;
}

Variable* SubWindow::FindVariable(const std::string& label) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& widget : widgets_) {
        auto variable = std::dynamic_pointer_cast<Variable>(widget);
        if (variable && variable->label() == label) {
            return variable.get();
        }
    }
    return nullptr;
}

const Variable* SubWindow::FindVariable(const std::string& label) const {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& widget : widgets_) {
        auto variable = std::dynamic_pointer_cast<Variable>(widget);
        if (variable && variable->label() == label) {
            return variable.get();
        }
    }
    return nullptr;
}

Structure& SubWindow::AddStructure(std::string label) {
    auto structure = std::make_shared<Structure>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    widgets_.push_back(structure);
    return *structure;
}

void SubWindow::Render() const {
    SubWindow::RenderCallback callback_copy;
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

SubWindow& SubWindowRegistry::Add(std::string name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto existing = FindLocked(name);
    if (existing) {
        return *existing;
    }

    auto window = std::make_shared<SubWindow>(std::move(name));
    auto& ref = *window;
    windows_.emplace(ref.name(), std::move(window));
    return ref;
}

SubWindow& SubWindowRegistry::operator[](const std::string& name) {
    return Add(std::string{name});
}

SubWindow* SubWindowRegistry::TryGet(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto window = FindLocked(name);
    return window.get();
}

const SubWindow* SubWindowRegistry::TryGet(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto window = FindLocked(name);
    return window.get();
}

std::vector<std::shared_ptr<SubWindow>> SubWindowRegistry::Snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<SubWindow>> snapshot;
    snapshot.reserve(windows_.size());
    for (const auto& [_, window] : windows_) {
        snapshot.push_back(window);
    }
    return snapshot;
}

std::shared_ptr<SubWindow> SubWindowRegistry::FindLocked(const std::string& name) const {
    auto it = windows_.find(name);
    if (it == windows_.end()) {
        return nullptr;
    }
    return it->second;
}

}  // namespace debugglass
