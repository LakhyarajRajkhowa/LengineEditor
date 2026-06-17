#include "AssetPanel.h"
#include <imgui.h>
#define THUMBNAIL_SIZE 64.0f
#define PADDING 12.0f

const float cellSize = THUMBNAIL_SIZE + PADDING;

namespace fs = std::filesystem;

using namespace Lengine;

AssetPanel::AssetPanel(const fs::path& root, AssetManager& asstMgr, ScriptSystem& scriptSys)
    : m_RootPath(root), m_CurrentPath(root),
    assetManager(asstMgr),
    scriptSystem(scriptSys)
{
    meshIcon = LoadThumbnail(Paths::Icons + "mesh_icon.png");
    prefabIcon = LoadThumbnail(Paths::Icons + "prefab_icon.png");
    submeshIcon = LoadThumbnail(Paths::Icons + "submesh_icon.png");
    textureIcon = LoadThumbnail(Paths::Icons + "texture_icon.png");
    materialIcon = LoadThumbnail(Paths::Icons + "material_icon.png");
    folderIcon = LoadThumbnail(Paths::Icons + "folder_icon.png");
    scriptIcon = LoadThumbnail(Paths::Icons + "script_icon.png");  
    skeletonIcon = LoadThumbnail(Paths::Icons + "skeleton_icon.png");
    boneMaskIcon = LoadThumbnail(Paths::Icons + "bone_mask_icon.jpg");

}

static AssetFolderView s_CurrentView = AssetFolderView::Root;
static char NewMaterialName[128] = "NewMatrial";
static bool openCreateMaterialPopup = false;
static char NewBoneMaskName[128] = "NewBoneMask";
static bool openCreateBoneMaskPopup = false;



void AssetPanel::OnImGuiRender()
{

    ImGui::Begin("Assets");

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, nullptr, false);

    if (s_CurrentView == AssetFolderView::Root)
    {
        DrawAssetTypeFolder("Prefab", folderIcon, AssetFolderView::Prefab);
        DrawAssetTypeFolder("SubMesh", folderIcon, AssetFolderView::Mesh);
        DrawAssetTypeFolder("Texture", folderIcon, AssetFolderView::Texture);
        DrawAssetTypeFolder("Material", folderIcon, AssetFolderView::PhongMaterial);
        DrawAssetTypeFolder("Scripts", folderIcon, AssetFolderView::Script); 
        DrawAssetTypeFolder("Skeleton", folderIcon, AssetFolderView::Skeleton);
        DrawAssetTypeFolder("Bone Mask", folderIcon, AssetFolderView::BoneMask);
    }
    else
    {
        DrawBackButton();
        switch (s_CurrentView)
        {
        case AssetFolderView::Mesh:          DrawSubMeshAssets();     break;
        case AssetFolderView::Prefab:        DrawPrefabAssets();      break;
        case AssetFolderView::Texture:       DrawTextureAssets();     break;
        case AssetFolderView::PhongMaterial: DrawPbrMaterialAssets(); break;
        case AssetFolderView::Script:        DrawScriptAssets();      break;
        case AssetFolderView::Skeleton:      DrawSkeletonAssets();      break;
        case AssetFolderView::BoneMask:      DrawBoneMaskAssets();      break;
        }
    }

    ImGui::Columns(1);
    ImGui::End();

    DrawCreateMaterialPopup();
    DrawCreateBoneMaskPopup();


}





void AssetPanel::CreateNewFolder(const fs::path& path)
{
    try
    {
        if (!fs::exists(path))
            fs::create_directory(path);
    }
    catch (...) {}
}


void AssetPanel::OpenImportMeshDialog(std::string folderPath)
{
    const char* filters[6] = { "*.obj", "*.fbx", "*.dae", "*.gltf", "*.glb", "*.blend" };

    const char* filePath = tinyfd_openFileDialog(
        "Import Mesh",
        "",
        6,
        filters,
        "Mesh Files",
        0
    );

    if (filePath)
    {
        assetManager.ImportMesh(filePath);
    }
}
void AssetPanel::OpenImportTextureDialog(const UUID& id)
{
    const char* filters[4] = { "*.png", "*.jpeg", "*.jpg", "*.tga"};

    const char* filePath = tinyfd_openFileDialog(
        "Import Texture",
        "",
        4,
        filters,
        "Texture Files",
        0
    );

    if (filePath)
    {
        assetManager.ImportTexture(filePath, id);
    }
}
void AssetPanel::OpenImportMaterialDialog()
{
    const char* filters[1] = { "*.pbrmat" };

    const char* filePath = tinyfd_openFileDialog(
        "Import Material",
        "",
        1,
        filters,
        "Material Files",
        0
    );

    if (filePath)
    {
        assetManager.ImportMaterial(filePath);
    }
}
void AssetPanel::OpenImportPrefabDialog(std::string folderPath)
{
    const char* filters[6] = { "*.obj", "*.fbx", "*.dae", "*.gltf", "*.glb", "*.blend" };

    const char* filePath = tinyfd_openFileDialog(
        "Prefab Mesh",
        "",
        6,
        filters,
        "Prefab Files",
        0
    );

    if (filePath)
    {
        assetManager.ImportPrefab(filePath);
    }
}

ImTextureID AssetPanel::LoadThumbnail(const std::string& file)
{
    if (thumbnailCache.count(file))
        return thumbnailCache[file];

    GLTexture* tex = assetManager.loadImage(file, file);
    if (!tex)
        return -1;

    ImTextureID id = (ImTextureID)(intptr_t)tex->id;
    thumbnailCache[file] = id;

    return id;
}

void AssetPanel::DrawAssetTypeFolder(
    const char* name,
    ImTextureID icon,
    AssetFolderView targetView
)
{
    ImGui::PushID(name);

    ImGui::Image(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

    if (ImGui::IsItemClicked())
        s_CurrentView = targetView;

    ImGui::TextWrapped("%s", name);

    ImGui::NextColumn();
    ImGui::PopID();
}

void AssetPanel::DrawBackButton()
{
    if (ImGui::Button("< Back"))
        s_CurrentView = AssetFolderView::Root;

    ImGui::Separator();
}

void AssetPanel::DrawSubMeshAssets()
{

    auto assets = assetManager.GetAllSubmeshFromDatabase();

    // Right click to import mesh inside this folder only
    if (ImGui::BeginPopupContextWindow("DirContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Import Mesh..."))
            m_OpenImportMeshDialog = true;

        ImGui::EndPopup();
    }

    if (m_OpenImportMeshDialog)
    {
        m_OpenImportMeshDialog = false;
        OpenImportMeshDialog(m_CurrentPath.string());
    }

    if (assets.empty())
    {
        ImGui::TextDisabled("No submesh assets imported.");
        return;
    }

    for (const auto& asset : assets)
    {
        ImGui::PushID((int)asset.uuid);
        ImGui::BeginGroup();
        // --- Define clickable region ---
        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##submesh_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            MeshDragPayload payload{};
            payload.id = asset.uuid;

            ImGui::SetDragDropPayload(
                "SUBMESH_ASSET",
                &payload,
                sizeof(payload)
            );
            ImGui::Image(submeshIcon, { 32, 32 });  
            ImGui::SameLine();
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        // Draw thumbnail
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(submeshIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // Draw label
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // --- Handle click ---
        if (clicked)
        {
            // TODO: store selected asset UUID
        }

        ImGui::EndGroup();
        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);

    
}

void AssetPanel::DrawPbrMaterialAssets()
{

    auto assets = assetManager.GetAllPbrMaterialFromDatabase();

    // Right click to import material inside this folder only
    if (ImGui::BeginPopupContextWindow("DirContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Import Material..."))
            m_OpenImportMaterialDialog = true;

        if (ImGui::MenuItem("Create New Material")) {
            openCreateMaterialPopup = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (m_OpenImportMaterialDialog)
    {
        m_OpenImportMaterialDialog = false;
        OpenImportMaterialDialog();
    }

    if (assets.empty())
    {
        ImGui::TextDisabled("No material assets imported.");
        return;
    }

    for (const auto& asset : assets)
    {
        // UUID = uin64 but pushID = int(32) so...
        ImGui::PushID((void*)(uintptr_t)asset.uuid);

        // --- Define clickable region ---
        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##material_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            MeshDragPayload payload{};
            payload.id = asset.uuid;

            ImGui::SetDragDropPayload(
                "PBRMATERIAL_ASSET",
                &payload,
                sizeof(payload)
            );
            ImGui::Image(materialIcon, { 32, 32 });   
            ImGui::SameLine();
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        // Draw thumbnail
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(materialIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // Draw label
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // --- Handle click ---
        if (clicked)
        {
            // TODO: store selected asset UUID
            EditorSelection::ClearAssetSelection();
            EditorSelection::SetAsset(std::pair(asset.uuid, AssetType::Material));

        }

        ImGui::NextColumn();
        ImGui::PopID();
    }
}

void AssetPanel::DrawSkeletonAssets()
{

    auto assets = assetManager.GetAllSkeletonsFromDatabase();

   
    if (assets.empty())
    {
        ImGui::TextDisabled("No skeleton assets imported.");
        return;
    }

    for (const auto& asset : assets)
    {
        // UUID = uin64 but pushID = int(32) so...
        ImGui::PushID((void*)(uintptr_t)asset.uuid);

        // --- Define clickable region ---
        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##skeleton_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            SkeletonDragPayload payload{};
            payload.id = asset.uuid;

            ImGui::SetDragDropPayload(
                "SKELETON_ASSET",
                &payload,
                sizeof(payload)
            );
            ImGui::Image(skeletonIcon, { 32, 32 });
            ImGui::SameLine();
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        // Draw thumbnail
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(skeletonIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // Draw label
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // --- Handle click ---
        if (clicked)
        {
            // TODO: store selected asset UUID
            EditorSelection::ClearAssetSelection();
            EditorSelection::SetAsset(std::pair(asset.uuid, AssetType::Skeleton));
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }
}


void AssetPanel::DrawBoneMaskAssets()
{

    auto assets = assetManager.GetAllBoneMasksFromDatabase();

    // Right click to import material inside this folder only
    if (ImGui::BeginPopupContextWindow("DirContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        
        if (ImGui::MenuItem("Create New Bone Mask")) {
            openCreateBoneMaskPopup = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }


    if (assets.empty())
    {
        ImGui::TextDisabled("No bone mask assets created.");
        return;
    }

    for (const auto& asset : assets)
    {
        // UUID = uin64 but pushID = int(32) so...
        ImGui::PushID((void*)(uintptr_t)asset.uuid);

        // --- Define clickable region ---
        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##bone_mask_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            BoneMaskDragPayload payload{};
            payload.id = asset.uuid;

            ImGui::SetDragDropPayload(
                "BONE_MASK_ASSET",
                &payload,
                sizeof(payload)
            );
            ImGui::Image(boneMaskIcon, { 32, 32 });
            ImGui::SameLine();
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        // Draw thumbnail
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(boneMaskIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // Draw label
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // --- Handle click ---
        if (clicked)
        {
            assetManager.LoadBoneMask(asset.uuid);
            EditorSelection::ClearAssetSelection();
            EditorSelection::SetAsset(std::pair(asset.uuid, AssetType::BoneMask));
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }
}

void AssetPanel::DrawTextureAssets()
{

    auto assets = assetManager.GetAllTexturesFromDatabase();

    // Right click to import material inside this folder only
    if (ImGui::BeginPopupContextWindow("DirContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Import Texture..."))
            m_OpenImportTextureDialog = true;

        ImGui::EndPopup();
    }

    if (m_OpenImportTextureDialog)
    {
        m_OpenImportTextureDialog = false;
        OpenImportTextureDialog(UUID());
    }

    if (assets.empty())
    {
        ImGui::TextDisabled("No texture assets imported.");
        return;
    }

    for (const auto& asset : assets)
    {
        ImGui::PushID((void*)(uintptr_t)asset.uuid);

        // --- Define clickable region ---
        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##texture_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            TextureDragPayload payload{};
            payload.id = asset.uuid;

            ImGui::SetDragDropPayload(
                "TEXTURE_ASSET",
                &payload,
                sizeof(payload)
            );
            ImGui::Image(textureIcon, { 32, 32 });  
            ImGui::SameLine();
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        // Draw thumbnail
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(textureIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // Draw label
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // --- Handle click ---
        if (clicked)
        {
            // TODO: store selected asset UUID
            EditorSelection::ClearAssetSelection();
            EditorSelection::SetAsset(std::pair(asset.uuid, AssetType::Texture));
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }
}


void AssetPanel::DrawPrefabAssets()
{

    auto assets = assetManager.GetAllPrefabsFromDatabase();

    // Right click to import mesh inside this folder only
    if (ImGui::BeginPopupContextWindow("DirContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Import Prefab..."))
            m_OpenImportPrefabDialog = true;

        ImGui::EndPopup();
    }

    if (m_OpenImportPrefabDialog)
    {
        m_OpenImportPrefabDialog = false;
        OpenImportPrefabDialog(m_CurrentPath.string());
    }

    if (assets.empty())
    {
        ImGui::TextDisabled("No prefab assets imported.");
        return;
    }

    for (const auto& asset : assets)
    {
        ImGui::PushID((int)asset.uuid);
        ImGui::BeginGroup();

        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        // --- Clickable item ---
        ImGui::InvisibleButton("##prefab_item", itemSize);

        bool leftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

        ImVec2 min = ImGui::GetItemRectMin();

        // --- Draw thumbnail ---
        ImGui::SetCursorScreenPos(min);
        ImGui::Image(prefabIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        // --- Draw label ---
        ImGui::SetCursorScreenPos({
            min.x,
            min.y + THUMBNAIL_SIZE
            });
        ImGui::TextWrapped("%s", asset.name.c_str());

        // ---------------- Context Menu ----------------
        if (ImGui::BeginPopupContextItem("PrefabContextMenu"))
        {
            if (ImGui::MenuItem("Load Prefab to scene"))
            {
                assetManager.LoadPrefabToScene(asset.libraryPath.string());
            }

            if (ImGui::MenuItem("Delete"))
            {
                // Optional
            }

            ImGui::EndPopup();
        }

        ImGui::EndGroup();
        ImGui::NextColumn();
        ImGui::PopID();
    }


    ImGui::Columns(1);


}

void AssetPanel::DrawScriptAssets()
{
    const auto& metadata = scriptSystem.GetLibrary().GetAllMetadata();

    if (metadata.empty())
    {
        ImGui::TextDisabled("No scripts registered.");
        return;
    }

    for (int i = 0; i < (int)metadata.size(); i++)
    {
        const auto& m = metadata[i];

        ImGui::PushID(i);

        ImVec2 itemSize = {
            THUMBNAIL_SIZE,
            THUMBNAIL_SIZE + ImGui::GetTextLineHeightWithSpacing()
        };

        bool clicked = ImGui::InvisibleButton("##script_item", itemSize);

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropSource())
        {
            ScriptDragPayload payload{};
            strncpy_s(payload.name, m.name.c_str(), sizeof(payload.name) - 1);
            strncpy_s(payload.sourceFile, m.sourceFile.c_str(), sizeof(payload.sourceFile) - 1);

            ImGui::SetDragDropPayload("SCRIPT_ASSET", &payload, sizeof(payload));
            ImGui::Image(scriptIcon, { 32, 32 });
            ImGui::SameLine();
            ImGui::Text("%s", m.name.c_str());
            ImGui::EndDragDropSource();
        }

        // --- Draw visuals ON TOP of invisible button ---
        ImVec2 min = ImGui::GetItemRectMin();

        ImGui::SetCursorScreenPos(min);
        ImGui::Image(scriptIcon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE });

        ImGui::SetCursorScreenPos({ min.x, min.y + THUMBNAIL_SIZE });
        ImGui::TextWrapped("%s", m.name.c_str());

        // --- Click ---
        if (clicked)
        {
            // TODO: open script in editor / show in inspector
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }
}
void AssetPanel::DrawCreateMaterialPopup()
{
    if (openCreateMaterialPopup)
    {
        ImGui::OpenPopup("Create New Material");
        openCreateMaterialPopup = false;
    }

    if (ImGui::BeginPopupModal("Create New Material", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Material Name");
        ImGui::InputText("##MaterialName", NewMaterialName, IM_ARRAYSIZE(NewMaterialName));
        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (strlen(NewMaterialName) > 0)
            {
                assetManager.CreateMaterial(NewMaterialName);

                strcpy_s(NewMaterialName, sizeof(NewMaterialName), "NewMaterial");

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void AssetPanel::DrawCreateBoneMaskPopup()
{
    if (openCreateBoneMaskPopup)
    {
        ImGui::OpenPopup("Create New Bone Mask");
        openCreateBoneMaskPopup = false;
    }

    if (ImGui::BeginPopupModal("Create New Bone Mask", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Bone Mask Name");
        ImGui::InputText("##BoneMaskName", NewBoneMaskName, IM_ARRAYSIZE(NewBoneMaskName));
        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (strlen(NewBoneMaskName) > 0)
            {
                assetManager.CreateBoneMask(NewBoneMaskName);

                strcpy_s(NewBoneMaskName, sizeof(NewBoneMaskName), "NewBoneMask");

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}