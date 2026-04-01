#pragma once

#include <imgui.h>

#include "resources/ImageLoader.h"
#include "graphics/geometry/HDREnvironment.h"

#include "external/tinyfiledialogs.h"

namespace Lengine {
    class EnvironmentPanel
    {
    public:

        void OnImGuiRender(HDREnvironment& hdrSkybox);
    private:


        void OpenImportTextureDialog(std::string folderPath);
     
    };

}

