#pragma once

#include <filesystem>
#include <string>
#include <vector>

// Editor
#include "EditorSelection.h"

// Engine
#include "utils/UUID.h"
#include "resources/AssetManager.h"
#include "external/tinyfiledialogs.h"
#include "scripting/ScriptSystem.h"

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

    struct ScriptDragPayload
    {
        char name[64];        
        char sourceFile[256]; 
    };
    enum class AssetFolderView
    {
        Root,       // showing folders
        Prefab,
        Mesh,
        Texture,
        PhongMaterial,
        Script
    };

    class AssetPanel {
    public:
        AssetPanel(const fs::path& root, AssetManager& asstMgr, ScriptSystem& scriptSys);

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
        void DrawScriptAssets();




    private:
        AssetManager& assetManager;
        ScriptSystem& scriptSystem;


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
        ImTextureID scriptIcon = 0;



    };

}
