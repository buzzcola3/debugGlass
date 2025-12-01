#include "debugglass/widgets/message_monitor.h"

#include <imgui.h>

#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace debugglass {
namespace {
constexpr float kHighlightWindowSeconds = 0.5f;

std::string FormatTimestamp(std::chrono::system_clock::time_point timestamp) {
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(timestamp);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - seconds).count();
    std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm_snapshot;
#if defined(_WIN32)
    localtime_s(&tm_snapshot, &tt);
#else
    localtime_r(&tt, &tm_snapshot);
#endif
    std::ostringstream stream;
    stream << std::put_time(&tm_snapshot, "%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms;
    return stream.str();
}
}

MessageMonitor::MessageMonitor(std::string label) : label_(std::move(label)) {}

void MessageMonitor::UpsertMessage(std::string id, std::string value) {
    auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = index_by_id_.find(id);
    if (found == index_by_id_.end()) {
        Entry entry;
        entry.id = std::move(id);
        entry.value = std::move(value);
        entry.update_count = 1;
        entry.timestamp = now;
        entries_.push_back(std::move(entry));
        index_by_id_[entries_.back().id] = entries_.size() - 1;
    } else {
        Entry& entry = entries_[found->second];
        entry.value = std::move(value);
        entry.update_count += 1;
        entry.timestamp = now;
    }
}

void MessageMonitor::Render() const {
    struct Snapshot {
        std::string id;
        std::string value;
        uint64_t update_count;
        std::chrono::system_clock::time_point timestamp;
    };

    std::vector<Snapshot> snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot.reserve(entries_.size());
        for (const auto& entry : entries_) {
            Snapshot snapshot_entry;
            snapshot_entry.id = entry.id;
            snapshot_entry.value = entry.value;
            snapshot_entry.update_count = entry.update_count;
            snapshot_entry.timestamp = entry.timestamp;
            snapshot.push_back(std::move(snapshot_entry));
        }
    }

    if (snapshot.empty()) {
        ImGui::TextUnformatted("No messages received");
        return;
    }

    bool copy_requested = false;
    const std::string copy_label = "Copy CSV##" + label_;
    if (ImGui::Button(copy_label.c_str())) {
        copy_requested = true;
    }

    const std::string table_id = "MessageMonitor##" + label_;
    const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_Reorderable;
    if (ImGui::BeginTable(table_id.c_str(), 4, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Updates", ImGuiTableColumnFlags_WidthFixed, 0.2f);
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed, 0.3f);
        ImGui::TableHeadersRow();

        const auto now = std::chrono::system_clock::now();
        for (const auto& entry : snapshot) {
            const auto age = now - entry.timestamp;
            const float age_seconds = std::chrono::duration_cast<std::chrono::duration<float>>(age).count();
            const bool highlight = age_seconds <= kHighlightWindowSeconds;

            if (highlight) {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);
                const ImU32 bg_color = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.25f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color);
            } else {
                ImGui::TableNextRow();
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(entry.id.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(entry.value.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%llu", static_cast<unsigned long long>(entry.update_count));

            ImGui::TableSetColumnIndex(3);
            const std::string timestamp_text = FormatTimestamp(entry.timestamp);
            ImGui::TextUnformatted(timestamp_text.c_str());
        }
        ImGui::EndTable();
    }

    if (copy_requested) {
        std::ostringstream csv;
        csv << "id,value,updates,timestamp\n";
        for (const auto& entry : snapshot) {
            csv << entry.id << ',' << entry.value << ','
                << entry.update_count << ',' << FormatTimestamp(entry.timestamp) << '\n';
        }
        const std::string csv_text = csv.str();
        ImGui::SetClipboardText(csv_text.c_str());
    }
}

}  // namespace debugglass
