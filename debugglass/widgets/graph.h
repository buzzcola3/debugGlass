#pragma once

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

#include "debugglass/widgets/window_content.h"

namespace debugglass {

class Graph : public WindowContent {
public:
    Graph(std::string label, std::size_t capacity = 256);

    void AddValue(float value);
    void SetRange(float min_value, float max_value);
    const std::string& label() const noexcept { return label_; }

    void Render() const override;

private:
    std::vector<float> CollectSamplesCopy() const;

    std::string label_;
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::vector<float> samples_;
    std::size_t next_index_ = 0;
    bool filled_ = false;
    float min_value_ = 0.0f;
    float max_value_ = 1.0f;
};

}  // namespace debugglass
