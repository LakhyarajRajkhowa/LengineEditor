#include "ConsolePanel.h"

using namespace Lengine;

void ConsolePanel::OnImGuiRender()
{
    ImGui::Begin("Console");

    static bool autoScroll = true;
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    ImGui::Separator();

    // ---- Build console buffer ----
    static std::vector<char> buffer;
    buffer.clear();

    const auto& logs = m_Buffer.GetLogs();
    for (const auto& msg : logs)
    {
        buffer.insert(buffer.end(), msg.begin(), msg.end());
        buffer.push_back('\n');
    }
    buffer.push_back('\0');

    // ---- Detect if we were already at bottom ----
    bool scrollToBottom =
        autoScroll &&
        (ImGui::GetScrollY() >= ImGui::GetScrollMaxY());

    // ---- Console text (selectable + copyable) ----
    ImGui::InputTextMultiline(
        "##console",
        buffer.data(),
        buffer.size(),
        ImVec2(-FLT_MIN, -FLT_MIN),
        ImGuiInputTextFlags_ReadOnly
    );

    // ---- Auto-scroll (only if user didn't scroll up) ----
    if (scrollToBottom)
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}

