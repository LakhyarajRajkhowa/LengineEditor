#pragma once

#include <external/json.hpp>

#include "core/Timer.h"
#include "core/settings.h"
#include "core/EventSystem.h"

#include "graphics/camera/Camera3d.h"
#include "graphics/renderer/RenderPipeline.h"
#include "graphics/renderer/PostProcess/PostProcessing.h"

#include "platform/InputManager.h"
#include "platform/Window.h"

#include "imgui/ImguiLayer.h"
#include "EditorLayer.h"

#include "graphics/PhysicsDebugRenderer.h"

namespace Lengine {
    class EditorOverlayPass
    {
    public:
        EditorOverlayPass(AssetManager& assets_ , PhysicsSystem& physics_) :
            assets(assets_),
            physics(physics_),
            gizmos(assets_),
            colliderRenderer(physics_)
        {
        }
        void InitGizmos() {
            gizmos.InitGizmo();
        }
        GizmoRenderer& getGizmos() { return gizmos; }
        PhysicsDebugRenderer& getPhysicsDebugRenderer() { return colliderRenderer; }

        void RenderGizmoGrid(RenderContext& ctx, Framebuffer& target)
        {
            target.Bind();

            glDisable(GL_CULL_FACE);

            gizmos.drawGizmoGrid(ctx.cameraView, ctx.cameraProjection, ctx.cameraPos);

            glEnable(GL_CULL_FACE);

            target.Unbind();
        }

        void RenderPhysicsCollider(RenderContext& ctx, Framebuffer& target)
        {
            target.Bind();

            glDisable(GL_CULL_FACE);

            colliderRenderer.Render(ctx);

            glEnable(GL_CULL_FACE);

            target.Unbind();
        }

    private:
        AssetManager& assets;
        PhysicsSystem& physics;

        GizmoRenderer gizmos;
        PhysicsDebugRenderer colliderRenderer;

    };

    class Editor
    {
    public:

        Editor(
            Window& window,
            InputManager& inputManager,
            EventSystem& eventSystem,
            AssetManager& assetManager,
            SceneManager& sceneManager,
            RenderSettings& renderSettings,
            RuntimeStats& runtimeStats,
            RenderPipeline& renderPipeline,
            PhysicsSystem& physSystem,
            bool& isRunning
        );

        void Init();
        void run(EditorMode& mode);
        void shutdown();

    private:

        Window& window;
        InputManager& inputManager;
        EventSystem& eventSystem;
        AssetManager& assetManager;
        SceneManager& sceneManager;
        RenderSettings& renderSettings;
        RuntimeStats& runtimeStats;
        RenderPipeline& renderPipeline;

        bool& isRunning;

        Camera3d editorCamera;

        ImGuiLayer imguiLayer;
        EditorLayer editorLayer;

        LogBuffer logBuffer;
        OutputRedirect* redirect = nullptr;

        EditorOverlayPass editorOverlays;

        PhysicsSystem& physSystem;
    
    };

   
}