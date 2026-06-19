#pragma once
#include <vector>
#include <string>
#include <queue>

// Editor
#include "Editor/core/EditorSelection.h"
#include "Editor/external/tinyfiledialogs.h"
#include "Editor/external/imgui/imgui.h"


// Engine
#include "graphics/camera/Camera3d.h"
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "resources/AssetManager.h"


namespace Lengine {
    


    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel(
            SceneManager& sceneManager,
            AssetManager& assetManager
        );

        void OnImGuiRender();
        void DrawEntityNode(Scene* scene, Entity entity, Scene* activeScene);
        void createNewModel();
        void drawCreateScenePopup();
        void drawRenameScenePopup();
    private:
        SceneManager& sceneManager;
        AssetManager& assetManager;
    private:

        Scene* activeScene;
    };

}
