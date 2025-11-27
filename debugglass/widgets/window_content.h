#pragma once

namespace debugglass {

class WindowContent {
public:
    virtual ~WindowContent() = default;
    virtual void Render() const = 0;
};

}  // namespace debugglass
