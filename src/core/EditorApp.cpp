#include "EditorApp.h"

namespace Lengine {

    Editor::Editor(
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
    )
        :

        window(window),
        inputManager(inputManager),
        eventSystem(eventSystem),
        assetManager(assetManager),
        sceneManager(sceneManager),
        renderSettings(renderSettings),
        runtimeStats(runtimeStats),
        renderPipeline(renderPipeline),
        isRunning(isRunning),

        imguiLayer(
            inputManager,
            isRunning,
            window.getWindow(),
            window.getGlContext()
        ),
        editorOverlays(assetManager, physSystem),
        editorLayer(
            window,
            logBuffer,
            sceneManager,
            editorOverlays.getGizmos(),
            editorCamera,
            inputManager,
            assetManager,
            renderSettings,
            runtimeStats,
            physSystem
        ),
        physSystem(physSystem)


    {
    }

    void Editor::Init()
    {
        editorCamera.Init(
            &inputManager
        );
        redirect = new OutputRedirect(logBuffer);

        eventSystem.addListener(
            [this](const SDL_Event& e)
            {
                imguiLayer.processEvent(e);
            }
        );

        editorOverlays.InitGizmos();
       
    }

    void Editor::run(EditorMode& mode)
    {

        imguiLayer.beginFrame();

        Scene* activeScene = sceneManager.GetActiveScene(mode);

        RenderContext ctx;
        ctx.scene = activeScene;
        ctx.settings = &renderSettings;

        if (mode == EditorMode::EDIT) {

           runtimeStats.frameStats =  LimitFPS(runtimeStats.targetFPS, runtimeStats.limitFPS);


            ctx.cameraView = editorCamera.getViewMatrix();
            ctx.cameraProjection = editorCamera.getProjectionMatrix();
            ctx.cameraPos = editorCamera.getCameraPosition();


            int mx, my;
            SDL_GetRelativeMouseState(&mx, &my);
            editorCamera.Update(runtimeStats.frameStats.deltaTime,  { mx, my });
        }
        else if (mode == EditorMode::PLAY) {
            UUID camID = activeScene->Cameras().GetPrimaryCameraID();

            glm::mat4 camView = glm::mat4(1.0f);
            glm::mat4 camProj = glm::mat4(1.0f);
            glm::vec3 camPos = glm::vec3(1.0f);

            if (camID != UUID::Null && activeScene->Transforms().Has(camID)) {

                camView = activeScene->Transforms().Get(camID).getViewMatrix();
                camProj = activeScene->Cameras().GetPrimaryCamera()->projection;
                camPos = activeScene->Transforms().Get(camID).GetWorldPosition();
            }
          
            ctx.cameraView = camView;
            ctx.cameraProjection = camProj;
            ctx.cameraPos = camPos;
        }


        
        renderPipeline.Render(ctx);

        editorOverlays.RenderGizmoGrid(ctx, renderPipeline.GetFinalFramebuffer());
     //   editorOverlays.RenderPhysicsCollider(ctx, renderPipeline.GetFinalFramebuffer());

        editorLayer.OnImGuiRender(renderPipeline.GetFinalImage(), renderPipeline.GetHDRSkybox(), mode);

        assetManager.drawLoadingScreens();
        assetManager.drawImportingScreens();

        imguiLayer.endFrame();
    }

    void Editor::shutdown()
    {
        imguiLayer.shutdown();

        if (redirect)
            delete redirect;
    }

}