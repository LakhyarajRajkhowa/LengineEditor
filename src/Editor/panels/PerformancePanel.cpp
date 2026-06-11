#include "PerformancePanel.h"

using namespace Lengine;

PerformancePanel::PerformancePanel(RuntimeStats& stats_): stats(stats_) {
	
}

void PerformancePanel::OnImGuiRender()
{
    ImGui::Begin("Performance");

    ImGui::Checkbox("Limit FPS", &stats.limitFPS);
    ImGui::SliderInt("Target FPS", &stats.targetFPS, 30, 240);


    // --- FPS smoothing ---
    constexpr float smoothing = 0.001f; // lower = smoother
    if (smoothedFPS == 0.0f)
    {
        smoothedFPS = stats.frameStats.fps;
        smoothedMs = stats.frameStats.msPerFrame;
    }
    else
    {
        smoothedFPS += (stats.frameStats.fps - smoothedFPS) * smoothing;
        smoothedMs += (stats.frameStats.msPerFrame - smoothedMs) * smoothing;
    }


    ImGui::Text("FPS: %.1f", smoothedFPS);
    ImGui::Text("Frame Time: %.2f ms", smoothedMs);

    ImGui::Separator();
    ImGui::End();
}
