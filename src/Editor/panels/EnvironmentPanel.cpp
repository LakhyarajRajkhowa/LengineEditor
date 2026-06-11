#include "EnvironmentPanel.h"

using namespace Lengine;

void EnvironmentPanel::OnImGuiRender(HDREnvironment& hdrSkybox){
    ImGui::Begin("Environment");

    ImGui::SliderFloat("Rotation", &hdrSkybox.envRotationAngle, 0.0f, 360.0f);

    ImGui::SliderFloat("Intensity", &hdrSkybox.envIntensity, 0.0f, 10.0f);

    ImGui::ColorEdit3("Tint", glm::value_ptr(hdrSkybox.envTint));


    ImGui::End();
}

void EnvironmentPanel::OpenImportTextureDialog(std::string folderPath)
{
    const char* filters[1] = { "*.hdr" };

    const char* filePath = tinyfd_openFileDialog(
        "HDRi",
        "",
        1,
        filters,
        "HDRi Files",
        0
    );

    if (filePath)
    {
        ImageLoader::LoadHDRTexture(filePath);
    }
}