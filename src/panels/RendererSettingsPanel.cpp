#include "RendererSettingsPanel.h"

using namespace Lengine;

void RendererSettingsPanel::OnImGuiRender()
{
    ImGui::Begin("Renderer Settings");

    // HDR
    //if (ImGui::Button(m_Settings.HDR ? "HDR: ON" : "HDR: OFF")) {
    //    m_Settings.HDR = !m_Settings.HDR;
    //    m_Settings.needsReload = true;
    //}


    ImGui::SliderFloat("Exposure", &m_Settings.exposure, 0.01f, 5.0f);

    if (ImGui::Button(m_Settings.enableBloom ? "Bloom: ON" : "Bloom: OFF")) {
        m_Settings.enableBloom = !m_Settings.enableBloom;
        m_Settings.needsReload = true;
    }

    if (m_Settings.enableBloom)
        ImGui::SliderFloat("Bloom Blur", &m_Settings.bloomBlur, 0.2f, 10.0f);
   

    ImGui::Separator();

    // MSAA (with reload marking)
    if (ImGui::Button(m_Settings.MSAA ? "MSAA: ON" : "MSAA: OFF"))
    {
        m_Settings.MSAA = !m_Settings.MSAA;
        m_Settings.needsReload = true;
    }

    if (m_Settings.MSAA)
    {
        static const int samples[] = { 2, 4, 8 };
        static const char* labels[] = { "2x", "4x", "8x" };

        int index = (m_Settings.msaaSamples == 8) ? 2 :
            (m_Settings.msaaSamples == 4) ? 1 : 0;

        if (ImGui::Combo("MSAA Samples", &index, labels, 3))
        {
            m_Settings.msaaSamples = samples[index];
            m_Settings.needsReload = true;
        }
    }

    ImGui::Separator();
    ImGui::Text("Debug View");

    if (ImGui::Checkbox("Enable Debug View", &IRenderer::enableDebugView))
    {
        // Optional: force something sensible
        if (IRenderer::enableDebugView)
            IRenderer::debugViewMode = DebugView::Geometry;
    }

    if (IRenderer::enableDebugView)
    {
        static const char* debugLabels[] = {
            "Geometry",
            "Albedo",
            "Normal",
            "Depth"
        };

        int currentMode = static_cast<int>(IRenderer::debugViewMode);

        if (ImGui::Combo("Mode", &currentMode, debugLabels, IM_ARRAYSIZE(debugLabels)))
        {
            IRenderer::debugViewMode = static_cast<DebugView>(currentMode);
        }
    }


    ImGui::End();
}
