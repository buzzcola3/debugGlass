#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "debugglass/widgets/window_content.h"

namespace debugglass {

class Variable;

class Structure : public WindowContent {
public:
    explicit Structure(std::string label);

    Structure& AddStructure(std::string label);
    Variable& AddVariable(std::string label);

    const std::string& label() const noexcept { return label_; }

    void Render() const override;

private:
    std::shared_ptr<Structure> AddStructureImpl(std::string label);
    std::shared_ptr<Variable> AddVariableImpl(std::string label);

    std::string label_;
    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<WindowContent>> children_;
};

}  // namespace debugglass
