#pragma once

#define IMGUI_ENABLE_DOCKING

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"

// Editor
#include "Editor/panels/MainMenuBar.h"
#include "Editor/panels/ViewportPanel.h"
#include "Editor/panels/SceneHeirarchyPanel.h"
#include "Editor/panels/InspectorPanel.h"
#include "Editor/panels/ConsolePanel.h"
#include "Editor/panels/AssetPanel.h"
#include "Editor/panels/PerformancePanel.h"
#include "Editor/panels/RendererSettingsPanel.h"
#include "Editor/panels/EnvironmentPanel.h"
#include "Editor/panels/AnimatorEditorPanel.h"
#include "Editor/graphics/Gizmos.h"


// Engine
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "graphics/geometry/ray.h"
#include "input/InputRouter.h"   
#include "input/KeyBindings.h"


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
            RuntimeStats& stats,
            PhysicsSystem& physSystem,
            InputRouter& inputRouter,
            ScriptSystem& scriptSystem
        );
        ~EditorLayer() = default;

        void OnAttach();
        void OnImGuiRender(
            const uint32_t& finalImage,
            const uint32_t& gbufferImage,
            HDREnvironment& hdrSkybox,
            EditorMode& mode
        );
        void OnDetach();

        ViewportPanel& GetViewportPanel() { return viewportPanel; }
        PerformancePanel& GetPerformancePanel() { return performancePanel; }
        MainMenuBar& GetMainMenuBar() { return mainMenuBar; }


    private:

        bool layoutInitialized = false;

        void BeginDockspace(EditorMode& mode);
        void SetupDefaultLayout();

    private:
        // Panels
        ViewportPanel viewportPanel;
        ViewportPanel testViewport;
        SceneHierarchyPanel hierarchyPanel;
        InspectorPanel inspectorPanel;
        ConsolePanel consolePanel;
        AssetPanel assetPanel;
        PerformancePanel performancePanel;
        RendererSettingsPanel rendererSettingsPanel;
        EnvironmentPanel environmentPanel;
        std::unordered_map<Entity, AnimatorEditorPanel> m_AnimatorPanels;
        MainMenuBar mainMenuBar;

        // External engine systems (not owned)
        Window& window;
        SceneManager& sceneManager;
        GizmoRenderer& gizmoRenderer;
        Camera3d& editorCamera;
        InputManager& inputManager;
        AssetManager& assetManager;
        RenderSettings& renderSettings;
        ScriptSystem& scriptSystem;

        PhysicsSystem& physSystem;
        InputRouter& inputRouter;   // NEW

        void OpenAnimatorEditor(Entity entity);

    };

}


