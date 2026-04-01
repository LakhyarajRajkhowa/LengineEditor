#pragma once

#define IMGUI_ENABLE_DOCKING

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"

#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "platform/KeyBindings.h"

#include "panels/ViewportPanel.h"
#include "panels/SceneHeirarchyPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/ConsolePanel.h"
#include "panels/AssetPanel.h"
#include "panels/PerformancePanel.h"
#include "panels/RendererSettingsPanel.h"
#include "panels/EnvironmentPanel.h"

#include "MainMenuBar.h"
#include "EditorManipulation.h"

#include "graphics/geometry/ray.h"
#include "graphics/geometry/Gizmos.h"

namespace Lengine {
   


    class EditorLayer {
    public:
        EditorLayer(
            Window& window,
            LogBuffer& buffer,
            SceneManager& sceneManager,
            GizmoRenderer& gizmoRndr,
            Camera3d& editorCamera,
            InputManager& inputManager,
            AssetManager& assetManager,
            RenderSettings& rndrSett,
            RuntimeStats& stats
            );
        ~EditorLayer() = default;

        void OnAttach();
        void OnImGuiRender(const uint32_t& finalImage, HDREnvironment& hdrSkybox);
        void OnDetach();

        ViewportPanel& GetViewportPanel() { return viewportPanel; }
        PerformancePanel& GetPerformancePanel() { return performancePanel; }
        
        EditorMode mode = EditorMode::EDIT;

    private:
        // Selection state
        Entity* selectedEntity = nullptr;
        Entity* hoveredEntity = nullptr;

        bool layoutInitialized = false;


        void BeginDockspace();
        void SetupDefaultLayout();

    private:
        // Panels
        ViewportPanel viewportPanel;
        SceneHierarchyPanel hierarchyPanel;
        InspectorPanel inspectorPanel;
        ConsolePanel consolePanel;
        AssetPanel assetPanel;
        PerformancePanel performancePanel;
        RendererSettingsPanel rendererSettingsPanel;
        EnvironmentPanel environmentPanel;

        MainMenuBar mainMenuBar;

        // External engine systems (not owned)
        Window& window;
        SceneManager& sceneManager;
        GizmoRenderer& gizmoRenderer;
        Camera3d& editorCamera;
        InputManager& inputManager;
        AssetManager& assetManager;
        RenderSettings& renderSettings;

    
    };

}


