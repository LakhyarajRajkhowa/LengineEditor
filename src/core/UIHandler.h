#pragma once

// Editor
#include "imgui/ImguiLayer.h"

// Engine
#include "input/InputRouter.h"


namespace Lengine {

    class UIHandler : public IInputHandler
    {
    public:
        UIHandler(ImGuiLayer& imgui) : m_imgui(imgui) {}

        void onEvent(const SDL_Event& event, InputManager& input) override
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

    private:
        ImGuiLayer& m_imgui;
    };

}