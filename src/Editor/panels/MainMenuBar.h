#pragma once
#include <imgui.h>
#include <fstream>
#include <filesystem>

// Engine
#include "core/settings.h"
#include "scene/SceneManager.h"

namespace fs = std::filesystem;
namespace Lengine {
    class MainMenuBar {
    public:
        MainMenuBar(SceneManager& sceneManager) :sceneManager(sceneManager) {}
        void OnImGuiRender(EditorMode& mode);
        std::function<void()> onPlayToggle;
    private:
        SceneManager& sceneManager;
        void DrawMainToolbar(EditorMode& mode);

    };
}
