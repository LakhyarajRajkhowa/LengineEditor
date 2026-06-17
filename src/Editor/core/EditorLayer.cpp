#include "EditorLayer.h"

// TODO : While loading Assets/Prefabs , that uses a different thread, turn off editor panels or freeze them

namespace Lengine {

    static bool isPlaying = false;
    static bool isPaused = false;

    EditorLayer::EditorLayer(
        Window& window,
        LogBuffer& buffer,
        SceneManager& scnMgr,
        GizmoRenderer& gizmoRndr,
        Camera3d& cam,
        InputManager& inputMgr,
        AssetManager& assetMgr,
        RenderSettings& rndrSett,
        RuntimeStats& stats_,
        PhysicsSystem& physSystem,
        InputRouter& router,
        ScriptSystem& scriptSystem
    )
        :
        window(window),
        editorCamera(cam),
        sceneManager(scnMgr),
        gizmoRenderer(gizmoRndr),
        inputManager(inputMgr),
        assetManager(assetMgr),
        renderSettings(rndrSett),
        inputRouter(router),
        scriptSystem(scriptSystem),
        hierarchyPanel(scnMgr, assetMgr),
        inspectorPanel(scnMgr, assetMgr, physSystem, scriptSystem),
        consolePanel(buffer),
        assetPanel(Paths::ActiveGameFolder, assetMgr, scriptSystem),
        rendererSettingsPanel(rndrSett),
        performancePanel(stats_),
        // Pass inputRouter to both viewports
        viewportPanel("Main Viewport", ViewportType::INTERACT, window, cam, scnMgr, router),
        testViewport("Test Viewport", ViewportType::READ_ONLY, window, cam, scnMgr, router),
        physSystem(physSystem),
        mainMenuBar(scnMgr)
    {
        inspectorPanel.onOpenAnimatorEditor = [this](Entity e)
            {
                OpenAnimatorEditor(e);
            };
    }

    void EditorLayer::OnAttach() {
        // nothing for now
    }

    void EditorLayer::OnDetach() {
        // cleanup if you want later
    }

    void EditorLayer::OnImGuiRender(
        const uint32_t& finalImage,
        const uint32_t& gbufferImage,
        HDREnvironment& hdrSkybox,
        EditorMode& mode
    ) {

        BeginDockspace(mode);


        //  Render panels
        if (!viewportPanel.viewportFullscreen) {

            viewportPanel.OnImGuiRender(finalImage);
            testViewport.OnImGuiRender(gbufferImage);
            hierarchyPanel.OnImGuiRender();
            inspectorPanel.OnImGuiRender();
            consolePanel.OnImGuiRender();
            assetPanel.OnImGuiRender();
            performancePanel.OnImGuiRender();
            rendererSettingsPanel.OnImGuiRender();
            environmentPanel.OnImGuiRender(hdrSkybox);

            for (auto it = m_AnimatorPanels.begin(); it != m_AnimatorPanels.end(); )
            {
                it->second.OnImGuiRender();
                if (!it->second.IsOpen())          // panel closed its own window
                    it = m_AnimatorPanels.erase(it);
                else
                    ++it;
            }
            

           

        }
        else {

            viewportPanel.RenderFullscreen(finalImage);
        }

    }

    // EditorLayer.cpp
    void EditorLayer::OpenAnimatorEditor(Entity entity)
    {
        Scene& scene = *sceneManager.GetEditorScene();
        Registry& reg = scene.GetRegistry();

        auto it = m_AnimatorPanels.find(entity);
        if (it == m_AnimatorPanels.end())
        {
            // Build a unique ImGui window title using the entity ID
            std::string title = "Animator Editor##" + std::to_string((uint64_t)entity);
            auto [inserted, _] = m_AnimatorPanels.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(entity),
                std::forward_as_tuple(reg, assetManager,  std::move(title))
            );
            inserted->second.SetTarget(entity);
        }
        else
        {
            // Already open — just re-focus / re-target in case component changed
            it->second.SetTarget(entity);
        }
    }

    // Dockspace
    void EditorLayer::BeginDockspace(EditorMode& mode)
    {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_MenuBar;

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("MainDockspace", nullptr, window_flags);

        ImGui::PopStyleVar(2);

        mainMenuBar.OnImGuiRender(mode);

        ImGuiID dockspace_id = ImGui::GetID("MainDockspaceID");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));

        ImGui::End();
    }


    // Default Layout 
    void EditorLayer::SetupDefaultLayout() {

        ImGuiID dockspace_id = ImGui::GetID("MainDockspaceID");

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        // Split into 3 columns: Hierarchy | Viewport | Inspector
        ImGuiID dock_left, dock_center, dock_right;

        // Left 20%
        dock_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.20f, nullptr, &dock_center);

        // Right 20%
        dock_right = ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Right, 0.20f, nullptr, &dock_center);

        // Optional: Bottom Console (25% height)
        ImGuiID dock_bottom;
        dock_bottom = ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Down, 0.25f, nullptr, &dock_center);

        // Assign windows
        ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
        ImGui::DockBuilderDockWindow("Inspector", dock_right);
        ImGui::DockBuilderDockWindow("Viewport", dock_center);
        ImGui::DockBuilderDockWindow("Console", dock_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }


}
