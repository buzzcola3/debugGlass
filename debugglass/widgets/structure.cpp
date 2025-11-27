#include "debugglass/widgets/structure.h"

#include <utility>

#include <imgui.h>

#include "debugglass/widgets/variable.h"

namespace debugglass {

Structure::Structure(std::string label) : label_(std::move(label)) {}

Structure& Structure::AddStructure(std::string label) {
    auto structure = AddStructureImpl(std::move(label));
    return *structure;
}

Variable& Structure::AddVariable(std::string label) {
    auto variable = AddVariableImpl(std::move(label));
    return *variable;
}

void Structure::Render() const {
    std::vector<std::shared_ptr<WindowContent>> children_snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        children_snapshot = children_;
    }

    ImGui::PushID(this);
    if (ImGui::TreeNode(label_.c_str())) {
        for (const auto& child : children_snapshot) {
            if (child) {
                child->Render();
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

std::shared_ptr<Structure> Structure::AddStructureImpl(std::string label) {
    auto structure = std::make_shared<Structure>(std::move(label));
    std::lock_guard<std::mutex> lock(mutex_);
    children_.push_back(structure);
    return structure;
}

std::shared_ptr<Variable> Structure::AddVariableImpl(std::string label) {
    auto variable = std::make_shared<Variable>(std::move(label));
    std::lock_guard<std::mutex> lock(mutex_);
    children_.push_back(variable);
    return variable;
}

}  // namespace debugglass
