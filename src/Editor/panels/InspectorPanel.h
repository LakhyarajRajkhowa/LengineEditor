#pragma once
#include <imgui.h>

// Editor
#include "AssetPanel.h"
#include "Editor/core/EditorSelection.h"

// Engine
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "physics/PhysicsSystem.h"
namespace Lengine {

    struct InspectorState {
        bool uniformScale = true;
    };


    class InspectorPanel {
    public:
        InspectorPanel(
            SceneManager& sceneManager,
            AssetManager& assetManager,
            PhysicsSystem& physSystem,
            ScriptSystem& scrSystem
        );

        void OnImGuiRender();


    private:
        SceneManager& sceneManager;
        AssetManager& assets;
        PhysicsSystem& physSystem;
        ScriptSystem& scriptSystem;

        void DrawEntityInspector(const Entity& entityID);
        void DrawAssetInspector(const std::pair<UUID, AssetType>& asset);
        void DrawMaterialEditor(const UUID& materialID);
        void DrawBoneMaskEditor(const UUID& materialID);
        void DrawEntityMaterialEditor(const Entity& entityID);

        InspectorState inspectorState;

        bool s_IsHovered = false;
        bool isAssetSaved = true;
        bool s_OpenSaveOrCancelPopup = false;

        void DrawSaveOrCancelPopup();
        void HandleAssetEditorClear();

        void DrawAddComponentMenu(
            const Entity& entityID,
            const Entity& entity,
            Registry& registry,
            Scene* scene);
        void DrawScriptComponent(const Entity& entityID, Registry& registry);




    };

}
