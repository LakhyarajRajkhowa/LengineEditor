#include "EditorApp.h"

namespace Lengine {

    Editor::Editor(
        Window& window,
        InputManager& inputManager,
        AssetManager& assetManager,
        SceneManager& sceneManager,
        RenderSettings& renderSettings,
        RuntimeStats& runtimeStats,
        RenderPipeline& renderPipeline,
        PhysicsSystem& physSystem,
        ScriptSystem& scriptSystem,
        InputRouter& inputRouter,
        bool& isRunning
    )
        :

        window(window),
        inputManager(inputManager),
        assetManager(assetManager),
        sceneManager(sceneManager),
        renderSettings(renderSettings),
        runtimeStats(runtimeStats),
        renderPipeline(renderPipeline),
        isRunning(isRunning),
        physSystem(physSystem),
        scriptSystem(scriptSystem),
        inputRouter(inputRouter),
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
            physSystem,
            inputRouter,
            scriptSystem
        ),
        uiHandler(imguiLayer),
        editorCameraHandler(editorCamera, inputRouter),
        gameHandler(sceneManager)
    {}

    void Editor::Init()
    {
        editorCamera.Init(
            &inputManager
        );
        redirect = new OutputRedirect(logBuffer);

        editorLayer.GetMainMenuBar().onPlayToggle = [this]() {
            if (mode == EditorMode::EDIT) {
                pressPlay();            
                mode = EditorMode::PLAY;

            }
            else {
                pressStop();           
                mode = EditorMode::EDIT;

            }
            };
        inputRouter.setGameHandler(&gameHandler);
        inputRouter.setEditorHandler(&editorCameraHandler);
        inputRouter.setUIHandler(&uiHandler);
        inputRouter.setContext(InputContext::UI);
        

        editorOverlays.InitGizmos();

    }

    void Editor::run()
    {

        imguiLayer.beginFrame();

        Scene* activeScene = sceneManager.GetActiveScene(mode);

        RenderContext ctx;
        ctx.scene = activeScene;
        ctx.settings = &renderSettings;

        if (mode == EditorMode::EDIT) {


            ctx.cameraView = editorCamera.getViewMatrix();
            ctx.cameraProjection = editorCamera.getProjectionMatrix();
            ctx.cameraPos = editorCamera.getCameraPosition();

            editorCamera.updateViewMatrix();

        }
        else if (mode == EditorMode::PLAY) {
            const Entity& camID = activeScene->GetPrimaryCamera();

            glm::mat4 camView = glm::mat4(1.0f);
            glm::mat4 camProj = glm::mat4(1.0f);
            glm::vec3 camPos = glm::vec3(1.0f);

            if (camID != NullEntity && activeScene->GetRegistry().transforms.Has(camID)) {

                const CameraComponent& cam = activeScene->GetRegistry().cameras.Get(camID);

                camView = activeScene->GetRegistry().transforms.Get(camID).getViewMatrix();
                camProj = cam.projection;
                camPos = activeScene->GetRegistry().transforms.Get(camID).GetWorldPosition();
            }

            ctx.cameraView = camView;
            ctx.cameraProjection = camProj;
            ctx.cameraPos = camPos;
        }



        renderPipeline.Render(ctx);

        editorOverlays.RenderGizmoGrid(ctx, renderPipeline.GetFinalFramebuffer());
        // editorOverlays.RenderPhysicsCollider(ctx, renderPipeline.GetFinalFramebuffer());

        editorLayer.OnImGuiRender(
            renderPipeline.GetFinalImage(),
            renderPipeline.GetGbufferNormal(),
            renderPipeline.GetHDRSkybox(),
            mode);

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

    void Editor::pressPlay()
    {

        sceneManager.CreateRuntimeScene();

        scriptSystem.OnCreate();
    }

    void Editor::pressStop()
    {
        scriptSystem.OnDestroy();

        sceneManager.GetRuntimeScene().reset();

    }

}