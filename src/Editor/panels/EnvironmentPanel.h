#pragma once


// Editor
#include "Editor/external/tinyfiledialogs.h"
#include "Editor/external/imgui/imgui.h"


// Engine
#include "resources/ImageLoader.h"
#include "graphics/geometry/HDREnvironment.h"

namespace Lengine {
    class EnvironmentPanel
    {
    public:

        void OnImGuiRender(HDREnvironment& hdrSkybox);
    private:


        void OpenImportTextureDialog(std::string folderPath);
     
    };

}

