#include <Windows.h>

#define SDL_MAIN_HANDLED


#include "core/EngineCore.h"
#include "core/EditorApp.h"

int main(int argc, char* argv[])
{
    EditorMode mode = EditorMode::EDIT;


    Lengine::EngineCore engine;
    engine.initSystems();

    Lengine::Editor editor(
        engine.getWindow(),
        engine.getInputManager(),
        engine.getEventSystem(),
        engine.getAssetManager(),
        engine.getSceneManager(),
        engine.getRenderSettings(),
        engine.getRuntimeStats(),
        engine.getRenderPipeline(),
        engine.getPhysicsSystem(),
        engine.isRunning()
    );
    editor.Init();

    while (engine.isRunning())
    {

        editor.run(mode);

        engine.run(mode);


        engine.presentFrame();
    }

    editor.shutdown();
    engine.shutdown();
}



