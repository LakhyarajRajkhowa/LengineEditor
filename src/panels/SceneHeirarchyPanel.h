#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <queue>
#include "graphics/camera/Camera3d.h"
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "resources/AssetManager.h"
#include "external/tinyfiledialogs.h"

#include "EditorSelection.h"

namespace Lengine {
    


    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel(
            SceneManager& sceneManager,
            AssetManager& assetManager
        );

        void OnImGuiRender();
        void DrawEntityNode(Scene* scene, Entity* entity, Scene* activeScene);
        void createNewModel();
        void drawCreateScenePopup();
        void drawRenameScenePopup();
    private:
        SceneManager& sceneManager;
        AssetManager& assetManager;
    private:
        std::queue<UUID> deletedEntityQueue;
        std::queue<std::pair<Entity*, UUID>> createdEntityQueue;

        Scene* activeScene;
    };

}
