#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "debugglass/widgets/graph.h"
#include "debugglass/widgets/structure.h"
#include "debugglass/widgets/variable.h"
#include "debugglass/widgets/window_content.h"

namespace debugglass {

class SubWindow {
public:
    using RenderCallback = std::function<void()>;

    explicit SubWindow(std::string name);

    const std::string& name() const noexcept { return name_; }

    void SetRenderCallback(RenderCallback callback);

    Graph& AddGraph(std::string label);
    Graph* FindGraph(const std::string& label);
    const Graph* FindGraph(const std::string& label) const;

    Variable& AddVariable(std::string label);
    Variable* FindVariable(const std::string& label);
    const Variable* FindVariable(const std::string& label) const;

    Structure& AddStructure(std::string label);

    void Render() const;

private:
    std::string name_;
    mutable std::mutex content_mutex_;
    RenderCallback callback_;
    std::vector<std::shared_ptr<WindowContent>> widgets_;
};

class SubWindowRegistry {
public:
    SubWindow& Add(std::string name);
    SubWindow& add(std::string name) { return Add(std::move(name)); }
    SubWindow& add(const char* name) { return Add(std::string{name}); }

    SubWindow& operator[](const std::string& name);

    SubWindow* TryGet(const std::string& name);
    const SubWindow* TryGet(const std::string& name) const;
    SubWindow* find(const std::string& name) { return TryGet(name); }
    const SubWindow* find(const std::string& name) const { return TryGet(name); }

    std::vector<std::shared_ptr<SubWindow>> Snapshot() const;

private:
    std::shared_ptr<SubWindow> FindLocked(const std::string& name) const;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<SubWindow>> windows_;
};

}  // namespace debugglass
