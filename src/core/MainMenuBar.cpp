#include "MainMenuBar.h"

using namespace Lengine;

void MainMenuBar::OnImGuiRender(EditorMode& mode) {
    DrawMainToolbar(mode);
}

void MainMenuBar::DrawMainToolbar(EditorMode& mode)
{
    if (ImGui::BeginMenuBar())
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 60.0f;
        float totalWidth = buttonWidth * 2 + 10;

        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);

        // PLAY / STOP
        if (ImGui::Button(mode == EditorMode::EDIT ? "Play" : "Stop"))
        {
            if (mode == EditorMode::EDIT) {
                mode = EditorMode::PLAY; 
                sceneManager.CreateRuntimeScene();
            }
            else {
                mode = EditorMode::EDIT;
                sceneManager.GetRuntimeScene().reset();
            }
        }

        ImGui::SameLine();

        // PAUSE / RESUME
        if (ImGui::Button(mode == EditorMode::PAUSE ? "Resume" : "Pause"))
        {
            if (mode == EditorMode::PLAY)
                mode = EditorMode::PAUSE;
            else if (mode == EditorMode::PAUSE)
                mode = EditorMode::PLAY;
        }

        ImGui::EndMenuBar();
    }
}