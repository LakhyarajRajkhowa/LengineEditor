#pragma once
#include <SDL2/SDL.h>
#define IMGUI_ENABLE_DOCKING

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

// Editor
#include "Editor/external/ImGuizmo.h"
#include "Editor/external/imnodes.h"

// Engine
#include "input/InputManager.h"

namespace Lengine {
	class ImGuiLayer {
	public:
		ImGuiLayer(
			InputManager& inputMgr,
			bool& run,
			SDL_Window* window,
			SDL_GLContext glContext
			) :
			inputManager(inputMgr),
			isRunning(run)
		{
			Init(window, glContext);
		};
		void Init(SDL_Window* window, SDL_GLContext glContext);
		void shutdown();
		void processEvent(const SDL_Event& e);
		void beginFrame();
		void endFrame();


		bool wantsCaptureMouse() const;
		bool wantsCaptureKeyboard() const;

		// Theme
		void SetModernDarkTheme();
	private:
		InputManager& inputManager;
		bool& isRunning;


	};
}
