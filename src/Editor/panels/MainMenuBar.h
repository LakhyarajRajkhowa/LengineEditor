#pragma once
#include <fstream>
#include <filesystem>

// Editor
#include "Editor/external/imgui/imgui.h"

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
