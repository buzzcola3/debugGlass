#include "debugglass/widgets/variable.h"

#include <utility>

#include <imgui.h>

namespace debugglass {

Variable::Variable(std::string label) : label_(std::move(label)) {}

void Variable::SetValue(const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ = value;
}

void Variable::SetValue(std::string&& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ = std::move(value);
}

void Variable::Render() const {
    std::string value_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        value_copy = value_;
    }
    ImGui::Text("%s: %s", label_.c_str(), value_copy.c_str());
}

}  // namespace debugglass
