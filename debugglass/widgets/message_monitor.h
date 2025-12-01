#pragma once

#include <chrono>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "debugglass/widgets/window_content.h"

namespace debugglass {

class MessageMonitor : public WindowContent {
public:
    explicit MessageMonitor(std::string label);

    const std::string& label() const noexcept { return label_; }

    void UpsertMessage(std::string id, std::string value);

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    void UpsertMessage(std::string id, T value) {
        std::ostringstream stream;
        if constexpr (std::is_floating_point_v<T>) {
            stream.setf(std::ios::fixed, std::ios::floatfield);
            stream.precision(3);
        }
        stream << value;
        UpsertMessage(std::move(id), stream.str());
    }

    void Render() const override;

private:
    struct Entry {
        std::string id;
        std::string value;
        uint64_t update_count = 0;
        std::chrono::system_clock::time_point timestamp;
    };

    std::string label_;
    mutable std::mutex mutex_;
    std::vector<Entry> entries_;
    std::unordered_map<std::string, std::size_t> index_by_id_;
};

}  // namespace debugglass
