#pragma once

// Editor
#include "Editor/external/imgui/imgui.h"

// Engine
#include "utils/fps.h"


namespace Lengine {
	class PerformancePanel {
	public:
		PerformancePanel(RuntimeStats& stats);
		
		void OnImGuiRender();


	private:
		RuntimeStats& stats;

		float smoothedFPS = 0.0f;
		float smoothedMs = 0.0f;

	};
}