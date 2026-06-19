#include "ImGuiLoadingScreen.h"

#include <iostream>

void ImGuiLoadingScreen::Show(
    const Lengine::LoadingScreenInfo& info)
{
    m_info = info;
    m_visible = true;
}

void ImGuiLoadingScreen::Hide()
{
    m_visible = false;
}

void ImGuiLoadingScreen::Render()
{
    if (!m_visible)
        return;

    std::cout << "......visible......\n";

    ImGuiViewport* vp =
        ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        vp->GetCenter(),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(
        ImVec2(360, 0),
        ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse;

    bool open = false;

    if (m_info.modal)
    {
        ImGui::OpenPopup(m_info.title.c_str());
        open =
            ImGui::BeginPopupModal(
                m_info.title.c_str(),
                nullptr,
                flags);
    }
    else
    {
        open =
            ImGui::Begin(
                m_info.title.c_str(),
                nullptr,
                flags);
    }

    if (!open)
        return;

    ImGui::TextWrapped(
        "%s",
        m_info.message.c_str());

    char overlay[32];

    std::snprintf(
        overlay,
        sizeof(overlay),
        "%d%%",
        int(m_info.progress * 100));

    ImGui::ProgressBar(
        m_info.progress,
        ImVec2(-1, 0),
        overlay);
    std::cout << ".....Progress bar.....\n";


    if (m_info.modal)
        ImGui::EndPopup();
    else
        ImGui::End();
}