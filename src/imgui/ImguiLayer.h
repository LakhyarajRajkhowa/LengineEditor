#pragma once
#include <SDL2/SDL.h>
#define IMGUI_ENABLE_DOCKING

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "external/ImGuizmo.h"
#include "platform/InputManager.h"

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
			init(window, glContext);
		};
		void init(SDL_Window* window, SDL_GLContext glContext);
		void shutdown();
		// process event and return true if ImGui consumed it (optional)
		void processEvent(const SDL_Event& e);
		void beginFrame();
		void endFrame();


		// helper
		bool wantsCaptureMouse() const;
		bool wantsCaptureKeyboard() const;

		// Theme
		void SetModernDarkTheme();
	private:
		InputManager& inputManager;
		bool& isRunning;


	};
}
