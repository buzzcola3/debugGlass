#pragma once

#include <mutex>
#include <sstream>
#include <string>

#include "debugglass/widgets/window_content.h"

namespace debugglass {

class Variable : public WindowContent {
public:
    explicit Variable(std::string label);

    void SetValue(const std::string& value);
    void SetValue(std::string&& value);

    template <typename T>
    void SetValue(T value) {
        std::ostringstream stream;
        stream << value;
        SetValue(stream.str());
    }

    const std::string& label() const noexcept { return label_; }

    void Render() const override;

private:
    std::string label_;
    mutable std::mutex mutex_;
    std::string value_;
};

}  // namespace debugglass
