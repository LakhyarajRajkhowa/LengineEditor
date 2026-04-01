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
        editorOverlays(assetManager),
        editorLayer(
            window,
            logBuffer,
            sceneManager,
            editorOverlays.getGizmos(),
            editorCamera,
            inputManager,
            assetManager,
            renderSettings,
            runtimeStats
        )


    {
    }

    void Editor::init()
    {
        editorCamera.init(
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

    void Editor::run()
    {


        imguiLayer.beginFrame();

        Scene* activeScene = sceneManager.getActiveScene();

        RenderContext ctx;
        ctx.scene = activeScene;
        ctx.settings = &renderSettings;

        if (editorLayer.mode == EditorMode::EDIT) {
            ctx.cameraView = editorCamera.getViewMatrix();
            ctx.cameraProjection = editorCamera.getProjectionMatrix();
            ctx.cameraPos = editorCamera.getCameraPosition();


            int mx, my;
            SDL_GetRelativeMouseState(&mx, &my);
            editorCamera.Update(runtimeStats.frameStats.deltaTime, { mx, my });
        }
        else if (editorLayer.mode == EditorMode::PLAY) {
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

        editorLayer.OnImGuiRender(renderPipeline.GetFinalImage(), renderPipeline.GetHDRSkybox());

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