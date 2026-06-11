#include <Windows.h>

#define SDL_MAIN_HANDLED

// Engine
#include "Editor/core/EditorApp.h"

// Editor
#include "core/EngineCore.h"

int main(int argc, char* argv[])
{


    Lengine::EngineCore engine;
    engine.initSystems();

    Lengine::Editor editor(
        engine.getWindow(),
        engine.getInputManager(),
        engine.getAssetManager(),
        engine.getSceneManager(),
        engine.getRenderSettings(),
        engine.getRuntimeStats(),
        engine.getRenderPipeline(),
        engine.getPhysicsSystem(),
        engine.getScriptSystem(),
        engine.getInputRouter(),
        engine.isRunning()
    );
    editor.Init();

    while (engine.isRunning())
    {

        editor.run();

        engine.run(editor.getMode());


        engine.presentFrame();
    }

    editor.shutdown();
    engine.shutdown();
}



