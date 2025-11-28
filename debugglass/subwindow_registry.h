#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "debugglass/widgets/tab.h"

namespace debugglass {

class SubWindow {
public:
    using RenderCallback = std::function<void()>;

    class TabCollection {
    public:
        explicit TabCollection(SubWindow& owner) : owner_(owner) {}

        Tab& add(std::string label) { return owner_.AddTab(std::move(label)); }
        Tab& Add(std::string label) { return owner_.AddTab(std::move(label)); }
        Tab* find(const std::string& label) { return owner_.FindTab(label); }
        const Tab* find(const std::string& label) const { return owner_.FindTab(label); }

    private:
        SubWindow& owner_;
    };

    explicit SubWindow(std::string name);

    const std::string& name() const noexcept { return name_; }

    TabCollection tabs;

    void SetRenderCallback(RenderCallback callback);

    Tab& AddTab(std::string label);
    Tab* FindTab(const std::string& label);
    const Tab* FindTab(const std::string& label) const;

    void Render() const;

private:
    std::string name_;
    mutable std::mutex content_mutex_;
    RenderCallback callback_;
    std::vector<std::shared_ptr<Tab>> tabs_;
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
