#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "utils/UUID.h"
#include "resources/AssetManager.h"
#include "EditorSelection.h"

#include "external/tinyfiledialogs.h"
namespace Lengine {
    struct MeshDragPayload
    {
        UUID id;
        char path[512];
    };
    struct TextureDragPayload
    {
        UUID id;
        char path[512];
    };
    struct MaterialDragPayload
    {
        UUID id;
        char path[512];
    };

    enum class AssetFolderView
    {
        Root,       // showing folders
        Prefab,
        Submesh,
        Texture,
        PhongMaterial
    };

    class AssetPanel {
    public:
        AssetPanel(const std::filesystem::path& root, AssetManager& assetMgr);

        void OnImGuiRender();


    private:
        void OpenImportMeshDialog(std::string folderPath);
        void OpenImportTextureDialog(const UUID& id);
        void OpenImportMaterialDialog();
        void OpenImportPrefabDialog(std::string folderpath);

        void DrawCreateMaterialPopup();
        void CreateNewFolder(const std::filesystem::path& path);

        ImTextureID LoadThumbnail(const std::string& file);

        void DrawAssetTypeFolder(
            const char* name,
            ImTextureID icon,
            AssetFolderView targetView
        );
        void DrawBackButton();

        void DrawSubMeshAssets();
        void DrawPbrMaterialAssets();
        void DrawTextureAssets();
        void DrawPrefabAssets();



    private:
        AssetManager& assetManager;

        std::unordered_map<std::string, ImTextureID> thumbnailCache;

        std::filesystem::path m_RootPath;
        std::filesystem::path m_CurrentPath;
        bool  m_OpenImportMeshDialog = false;
        bool  m_OpenImportTextureDialog = false;
        bool  m_OpenImportMaterialDialog = false;
        bool  m_OpenImportPrefabDialog = false;



        char m_FolderNameBuffer[256] = "";
        bool m_ShowCreateFolderPopup = false;

        bool isSelected = false;
    private:

        ImTextureID folderIcon = 0;
        ImTextureID meshIcon = 0;
        ImTextureID prefabIcon = 0;
        ImTextureID submeshIcon = 0;
        ImTextureID textureIcon = 0;
        ImTextureID materialIcon = 0;



    };

}
