#pragma once

#include <imgui/imgui.h>

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