#pragma once

//editor
#include "Editor/external/imgui/imgui.h"

// engine
#include "resources/ILoadingScreen.h"

class ImGuiLoadingScreen :
    public Lengine::ILoadingScreen
{
public:

    void Show(
        const Lengine::LoadingScreenInfo& info
    ) override;

    void Hide() override;

    void Render();

private:

    bool m_visible = false;

    Lengine::LoadingScreenInfo m_info;
};