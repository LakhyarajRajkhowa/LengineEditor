#pragma once

#include "scene/SceneManager.h"
#include "resources/AssetManager.h"
#include "graphics/camera/Camera3d.h"
#include "panels/ViewportPanel.h"

#include "platform/InputManager.h"
#include "platform/KeyBindings.h"


#include "graphics/geometry/ray.h"
#include "graphics/geometry/Gizmos.h"

namespace Lengine {
    class EditorManipulator
    {
    public:

        EditorManipulator(
            SceneManager& sceneManager,
            AssetManager& assetManager,
            Camera3d& editorCamera,
            ViewportPanel& viewportPanel,
            InputManager& inputManager
            ) :
            sceneManager(sceneManager),
            assetManager(assetManager),
            editorCamera(editorCamera),
            viewportPanel(viewportPanel),
            inputManager(inputManager)
            
        {}

        void CheckForHoveredEntity();
        void SelectHoveredEntity();
        void HandleKeyboardShortcuts(const SDL_Keycode& key);

        void DeselectAllEntites() {
            EditorSelection::ClearEntitySelection();
        }

        bool IsAnyEntitySelected() {
            return EditorSelection::HasEntity();
        }


        Entity* GetHoveredEntity() const { return hoveredEntity; }
        Entity* GetSelectedEntity() const { return selectedEntity; }

    private:

        SceneManager& sceneManager;
        AssetManager& assetManager;
        Camera3d& editorCamera;
        ViewportPanel& viewportPanel;
        InputManager& inputManager;

        Entity* hoveredEntity = nullptr;
        Entity* selectedEntity = nullptr;

        float movementSpeed = 5.0f;
    };
}
