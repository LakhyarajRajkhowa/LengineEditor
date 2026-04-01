#pragma once
#include <imgui.h>
#include <fstream>
#include <filesystem>

#include "core/settings.h"

namespace fs = std::filesystem;
namespace Lengine {
    class MainMenuBar {
    public:
        void OnImGuiRender(EditorMode& mode);
    private:
        void DrawMainToolbar(EditorMode& mode);

    };
}
