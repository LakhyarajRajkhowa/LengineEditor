#pragma once

// Editor
#include "Editor/external/imgui/imgui.h"

// Engine
#include "core/settings.h"
#include "graphics/renderer/IRenderer.h"

namespace Lengine {
    class RendererSettingsPanel
    {
    public:
        RendererSettingsPanel(RenderSettings& settings)
            : m_Settings(settings) {
        }

        void OnImGuiRender();

    private:
        RenderSettings& m_Settings;
    };
    
}

