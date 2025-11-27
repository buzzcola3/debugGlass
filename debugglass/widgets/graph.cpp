#include "debugglass/widgets/graph.h"

#include <algorithm>
#include <utility>

#include <imgui.h>

namespace debugglass {

Graph::Graph(std::string label, std::size_t capacity)
    : label_(std::move(label)), capacity_(std::max<std::size_t>(2, capacity)) {
    samples_.reserve(capacity_);
}

void Graph::AddValue(float value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (samples_.size() < capacity_) {
        samples_.push_back(value);
        if (samples_.size() == capacity_) {
            filled_ = true;
            next_index_ = 0;
        }
    } else {
        samples_[next_index_] = value;
        next_index_ = (next_index_ + 1) % capacity_;
        filled_ = true;
    }
}

void Graph::SetRange(float min_value, float max_value) {
    if (min_value > max_value) {
        std::swap(min_value, max_value);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    min_value_ = min_value;
    max_value_ = max_value;
}

void Graph::Render() const {
    const auto samples = CollectSamplesCopy();
    if (samples.empty()) {
        ImGui::TextUnformatted("No samples yet");
        return;
    }

    ImGui::PlotLines(label_.c_str(), samples.data(), static_cast<int>(samples.size()), 0, nullptr, min_value_, max_value_, ImVec2(0.0f, 120.0f));
}

std::vector<float> Graph::CollectSamplesCopy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (samples_.empty()) {
        return {};
    }

    if (!filled_) {
        return samples_;
    }

    std::vector<float> ordered(capacity_);
    std::size_t idx = 0;
    for (std::size_t i = next_index_; i < capacity_; ++i) {
        ordered[idx++] = samples_[i];
    }
    for (std::size_t i = 0; i < next_index_; ++i) {
        ordered[idx++] = samples_[i];
    }
    return ordered;
}

}  // namespace debugglass
