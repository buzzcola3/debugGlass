#include "debugglass/subwindow_registry.h"

#include <utility>

#include <imgui.h>


namespace debugglass {

SubWindow::SubWindow(std::string name) : name_(std::move(name)), tabs(*this) {}

void SubWindow::SetRenderCallback(RenderCallback callback) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    callback_ = std::move(callback);
}

Tab& SubWindow::AddTab(std::string label) {
    auto tab = std::make_shared<Tab>(std::move(label));
    std::lock_guard<std::mutex> lock(content_mutex_);
    tabs_.push_back(tab);
    return *tab;
}

Tab* SubWindow::FindTab(const std::string& label) {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& tab : tabs_) {
        if (tab && tab->label() == label) {
            return tab.get();
        }
    }
    return nullptr;
}

const Tab* SubWindow::FindTab(const std::string& label) const {
    std::lock_guard<std::mutex> lock(content_mutex_);
    for (const auto& tab : tabs_) {
        if (tab && tab->label() == label) {
            return tab.get();
        }
    }
    return nullptr;
}

void SubWindow::Render() const {
    SubWindow::RenderCallback callback_copy;
    std::vector<std::shared_ptr<Tab>> tabs_snapshot;
    {
        std::lock_guard<std::mutex> lock(content_mutex_);
        callback_copy = callback_;
        tabs_snapshot = tabs_;
    }

    if (callback_copy) {
        callback_copy();
    }

    if (tabs_snapshot.empty()) {
        ImGui::TextUnformatted("No tabs defined");
        return;
    }

    std::string tab_bar_id = "Tabs##" + name_;
    if (ImGui::BeginTabBar(tab_bar_id.c_str())) {
        for (const auto& tab : tabs_snapshot) {
            if (!tab) {
                continue;
            }
            const std::string& label = tab->label();
            const char* tab_title = label.empty() ? "Tab" : label.c_str();
            if (ImGui::BeginTabItem(tab_title)) {
                tab->Render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
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
