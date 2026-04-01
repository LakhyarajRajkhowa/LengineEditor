#pragma once
#include <imgui.h>

#include "AssetPanel.h"

#include "scene/Scene.h"
#include "scene/SceneManager.h"

#include "EditorSelection.h"
namespace Lengine {

    struct InspectorState {
        bool uniformScale = true;
    };


    class InspectorPanel {
    public:
        InspectorPanel(
            SceneManager& sceneManager,
            AssetManager& assetManager
        );

        void OnImGuiRender();


    private:

        void DrawEntityInspector(const UUID& entityID);
        void DrawAssetInspector(const std::pair<UUID, AssetType>& asset);

        void DrawMaterialEditor(const UUID& materialID);
        void DrawEntityMaterialEditor(const UUID& entityID);


        SceneManager& sceneManager;
        AssetManager& assets;

        InspectorState inspectorState;

        bool s_IsHovered = false;
        bool isAssetSaved = true;
        bool s_OpenSaveOrCancelPopup = false;

        void DrawSaveOrCancelPopup();
        void HandleAssetEditorClear();

    };

}
