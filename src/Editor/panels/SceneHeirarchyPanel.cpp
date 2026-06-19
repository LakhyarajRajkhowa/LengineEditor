#include "SceneHeirarchyPanel.h"



using namespace Lengine;
SceneHierarchyPanel::SceneHierarchyPanel(
    SceneManager& scnMgr,
    AssetManager& assetMgr
)
    :
    sceneManager(scnMgr),
    assetManager(assetMgr)
{
}

static bool openCreateScenePopup = false;
static char NewSceneName[128] = "NewScene";
static bool openRenameScenePopup = false;
static char RenameSceneBuffer[128];
static Scene* sceneToRename = nullptr;

void SceneHierarchyPanel::OnImGuiRender() {
    ImGui::Begin("Hierarchy");

    auto& allScenes = sceneManager.getScenes();
    Scene* activeScene = sceneManager.GetEditorScene();

    ImGuiTreeNodeFlags rootFlags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanFullWidth;

    bool rootOpen = ImGui::TreeNodeEx("Scenes", rootFlags);

    // ---- RIGHT CLICK ON ROOT NODE ----
    if (ImGui::BeginPopupContextItem("ScenesRootPopup"))
    {
        if (ImGui::MenuItem("Create Scene"))
        {
            openCreateScenePopup = true;
        }
        ImGui::EndPopup();
    }

    // Attach popup to last item
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("ScenesRootPopup");
    }


    // Root node
    if (rootOpen)
    {
        for (auto& scene : allScenes)
        {
            
            // Highlight active scene
            ImGuiTreeNodeFlags flags =
                (scene.get() == activeScene ? ImGuiTreeNodeFlags_DefaultOpen : 0);

            bool sceneOpen = ImGui::TreeNodeEx(
                scene->getName().c_str(),
                flags | ImGuiTreeNodeFlags_OpenOnArrow
            );

            static bool openModelPopup = false;

            // --- RIGHT CLICK MENU FOR SCENE ---
            if (ImGui::BeginPopupContextItem(scene->getName().c_str()))
            {
                if (ImGui::MenuItem("Set Active")) {
                    sceneManager.setActiveScene(scene.get());
                    EditorSelection::ClearEntitySelection();
                    
                }

                
                if (scene.get() == activeScene) {
                    if (ImGui::MenuItem("Save Scene")) {
                        assetManager.saveScene(*scene, Paths::GameScenes);
                        //assetManager.saveSceneAssetRegistryForScene(*scene, Paths::GameAssetRegistryFolder);
                    }

                    if (ImGui::MenuItem("Add Entity")) {
                        ImGui::OpenPopup("Add New Entity");
                        openModelPopup = true;
                    }    
                }

                if (ImGui::MenuItem("Rename Scene"))
                {
                    sceneToRename = scene.get();
                    strncpy_s(RenameSceneBuffer,
                        scene->getName().c_str(),
                        IM_ARRAYSIZE(RenameSceneBuffer));
                    openRenameScenePopup = true;
                }
               

                ImGui::EndPopup();
            }

            // Modal for new entity creation
            if (openModelPopup) {
                ImGui::OpenPopup("Add New Entity");
                openModelPopup = false;
            }
            createNewModel();

            // --- ENTITIES UNDER THAT SCENE ---
            if (sceneOpen)
            {
               

                // ----- ENTITIES -----
                for (auto& entityID : scene->GetRootEntities())
                {
                    Entity entity = entityID;

                    if (scene.get() == activeScene)
                        DrawEntityNode(scene.get(), entity, activeScene);
                    else
                        ImGui::Text("%s", scene->GetRegistry().nameTags.Get(entity).name.c_str());
                }


                ImGui::TreePop();    
            }
            
        }

        ImGui::TreePop();
    }

   

    ImGui::End();

    drawRenameScenePopup();
    drawCreateScenePopup();

}

void SceneHierarchyPanel::DrawEntityNode(Scene* scene, Entity entity, Scene* activeScene)
{
    if (!entity)
        return;

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanFullWidth;

    bool isSelected =
        EditorSelection::HasEntity() &&
        EditorSelection::GetEntity() == entity;

    if (isSelected)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Leaf node?
    if (!scene->HasChildren(entity))
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID((int)entity);

    bool opened = ImGui::TreeNodeEx(
        scene->GetRegistry().nameTags.Get(entity).name.c_str(),
        flags
    );

    

    // ---- Selection ----
    if (ImGui::IsItemClicked())
    {
        EditorSelection::SetEntity(entity);
        EditorSelection::ClearAssetSelection();
    }

    // ---- Context menu ----
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Create Copy"))
        {
            scene->DuplicateHierarchy(entity);
        }

        if (ImGui::MenuItem("Copy Entity ID"))
        {
            EditorSelection::SetCopiedEntity(entity);
        }

        bool hasParent = false;

        if (scene->GetRegistry().hierarchies.Has(entity))
        {
            auto& h = scene->GetRegistry().hierarchies.Get(entity);
            hasParent = (h.parent != NullEntity);
        }

        if (hasParent)
        {
            if (ImGui::MenuItem("Cut From Parent"))
            {
                scene->MakeOrphan(entity);
            }
        }

        if (ImGui::MenuItem("Delete"))
        {
            if (isSelected)
                EditorSelection::ClearEntitySelection();

            scene->DestroyEntity(entity);
        }

        ImGui::EndPopup();
    }

    // ---- Children ----
    if (opened && scene->HasChildren(entity))
    {
        for (Entity childID : scene->GetChildren(entity))
        {
            DrawEntityNode(scene, childID, activeScene);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}




void SceneHierarchyPanel::createNewModel() {
    Scene* activeScene = sceneManager.GetEditorScene();

    static char EntityName[128] = "MyEntity";
    static int entityTypeIndex = 0; // default selected type index

    if (ImGui::BeginPopupModal("Add New Entity", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter Entity Name:");
        ImGui::InputText("##name", EntityName, IM_ARRAYSIZE(EntityName));
        ImGui::Separator();


        if (ImGui::Button("Create"))
        {
            if (strlen(EntityName) > 0) {
                // Convert index to EntityType enum

                Entity newEntity = activeScene->CreateEntity_root(
                    EntityName    
                );

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneHierarchyPanel::drawCreateScenePopup()
{
    if (openCreateScenePopup)
    {
        ImGui::OpenPopup("Create New Scene");
        openCreateScenePopup = false;
    }

    if (ImGui::BeginPopupModal(
        "Create New Scene",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Scene Name");

        ImGui::InputText(
            "##SceneName",
            NewSceneName,
            IM_ARRAYSIZE(NewSceneName));

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            if (strlen(NewSceneName) > 0)
            {
                auto newScene =
                    assetManager.createScene(
                        NewSceneName,
                        Paths::GameScenes);

                Scene* scenePtr = newScene.get();

                sceneManager.getScenes().push_back(
                    std::move(newScene));


                strcpy_s(NewSceneName, "NewScene");

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
void SceneHierarchyPanel::drawRenameScenePopup()
{
    if (openRenameScenePopup)
    {
        ImGui::OpenPopup("Rename Scene");
        openRenameScenePopup = false;
    }

    if (ImGui::BeginPopupModal("Rename Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("New Scene Name");
        ImGui::InputText("##RenameScene", RenameSceneBuffer, IM_ARRAYSIZE(RenameSceneBuffer));
        ImGui::Separator();

        if (ImGui::Button("Rename", ImVec2(120, 0)))
        {
            if (sceneToRename && strlen(RenameSceneBuffer) > 0)
            {
               

                sceneToRename->rename(RenameSceneBuffer);

                ImGui::CloseCurrentPopup();
                sceneToRename = nullptr;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            sceneToRename = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

