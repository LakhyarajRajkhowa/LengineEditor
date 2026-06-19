#include "InspectorPanel.h"

using namespace Lengine;

InspectorPanel::InspectorPanel(
    SceneManager& scnMgr,
    AssetManager& asstMgr,
    PhysicsSystem& physSystem,
    ScriptSystem& scriptSys
) :
    sceneManager(scnMgr),
    assets(asstMgr),
    physSystem(physSystem),
    scriptSystem(scriptSys)

{}


void InspectorPanel::OnImGuiRender() {
    ImGui::Begin("Inspector");

    s_IsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

    ImGui::Separator();

    if (EditorSelection::HasAsset()) {
        DrawAssetInspector(EditorSelection::GetAsset());
    }
    else if (EditorSelection::HasEntity()) {
        DrawEntityInspector(EditorSelection::GetEntity());
    }

    ImGui::End();

    HandleAssetEditorClear();
    DrawSaveOrCancelPopup();
}

void InspectorPanel::DrawAssetInspector(const std::pair<UUID, AssetType>& asset) {
    if (asset.second == AssetType::Material) {
        DrawMaterialEditor(asset.first);
    }

    if (asset.second == AssetType::BoneMask) {
        DrawBoneMaskEditor(asset.first);
    }
}


void InspectorPanel::DrawMaterialEditor(const UUID& id)
{
    Material* mat = assets.GetMaterial(id);
    if (!mat)
    {
        ImGui::TextDisabled("Material not loaded");
        ImGui::Separator();

        if (ImGui::Button("Load")) {
            assets.LoadMaterial(id);
        }
        return;
    }

    ImGui::Separator();
    ImGui::Text("PBR Material");
    ImGui::Spacing();

    auto DrawVec3 = [&](const char* label, glm::vec3& value)
        {
            if (ImGui::ColorEdit3(label, glm::value_ptr(value)))
            {
                mat->localDirty = true;
            }
            ImGui::Spacing();
        };

    auto DrawFloat = [&](const char* label,
        float& value,
        float speed,
        float min,
        float max)
        {
            if (ImGui::DragFloat(label, &value, speed, min, max))
            {
                mat->localDirty = true;
            }
            ImGui::Spacing();
        };

    auto DrawTextureSlot = [&](const char* label,
        TextureMapType textMapType,
        bool srgb,
        UUID& slot)
        {
            std::string texName = "None";

            if (slot != UUID::Null)
            {
                texName = assets.GetAssetMetaData(slot)->name;
            }

            ImGui::Text(label);

            ImGui::Button(
                (texName + std::string("##") + label).c_str(),
                { ImGui::GetContentRegionAvail().x, 28 }
            );

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("TEXTURE_ASSET"))
                {
                    const TextureDragPayload* data =
                        static_cast<const TextureDragPayload*>(payload->Data);

                    if (!assets.getTexture(data->id))
                    {
                        assets.RequestTextureLoad(data->id, mat->id, textMapType, srgb);
                    }

                    mat->localDirty = true;
                }
                ImGui::EndDragDropTarget();
            }

            if (slot != UUID::Null &&
                ImGui::SmallButton(("Clear##" + std::string(label)).c_str()))
            {
                slot = UUID::Null;
                mat->localDirty = true;
            }

            ImGui::Spacing();
        };

    DrawVec3("Albedo", mat->albedo);

    DrawFloat("Ambient Occlusion", mat->ao, 0.01f, 0.0f, 1.0f);
    DrawFloat("Metallic", mat->metallic, 0.01f, 0.0f, 1.0f);
    DrawFloat("Roughness", mat->roughness, 0.01f, 0.0f, 1.0f);
    DrawFloat("Normal Strength", mat->normalStrength, 0.1f, -100.0f, 100.0f);
    DrawFloat("Opacity", mat->opacity, 0.01f, 0.0f, 1.0f);

    ImGui::Spacing();
    if (ImGui::Checkbox("Transparent", &mat->isTransparent))
    {
        mat->localDirty = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(Routes object through transparent pass)");
    ImGui::Spacing();

    ImGui::Separator();


    ImGui::Separator();

    DrawTextureSlot("Albedo Map", TextureMapType::Albedo, true, mat->map_albedo);
    DrawTextureSlot("Normal Map", TextureMapType::Normal, false, mat->map_normal);
    DrawTextureSlot("Ambient Occlusion Map", TextureMapType::AmbientOcclusion, false, mat->map_ao);
    DrawTextureSlot("Metallic Map", TextureMapType::Metallic, false, mat->map_metallic);
    DrawTextureSlot("Roughness Map", TextureMapType::Roughness, false, mat->map_roughness);
    DrawTextureSlot("Metallic Roughness Map", TextureMapType::MetallicRoughness, false, mat->map_metallicRoughness);

    if (mat->localDirty)
    {
        if (ImGui::Button("Save")) {
            assets.SaveMaterial(id);
            mat->localDirty = false;
            isAssetSaved = true;
        }

        isAssetSaved = false;
    }
}

void InspectorPanel::DrawBoneMaskEditor(const UUID& id)
{
    auto boneMask = assets.GetBoneMask(id);

    if (!boneMask)
    {
        ImGui::TextDisabled("Bone Mask failed to load");
        return;
    }

    ImGui::Text("Name: %s", boneMask->name.c_str());

    ImGui::Separator();



    // =====================================================
    // No skeleton assigned
    // =====================================================

    static bool skeletonPayloadDropped = false;

    if (boneMask->skeletonId == UUID::Null)
    {
        ImGui::Text("Skeleton");

        ImGui::Button(skeletonPayloadDropped ? "Skeleton asset missing" : "Drop Skeleton Here", { 250, 40 });

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("SKELETON_ASSET"))
            {
                skeletonPayloadDropped = true;
                const SkeletonDragPayload* data =
                    static_cast<const SkeletonDragPayload*>(payload->Data);


                Skeleton* skeleton =
                    assets.GetSkeleton(data->id);

                if (skeleton)
                {
                    boneMask->skeletonId = data->id;
                    boneMask->boneNames.clear();
                    boneMask->boneMask.clear();

                    boneMask->boneNames.resize(
                        skeleton->bones.size());

                    boneMask->boneMask.resize(
                        skeleton->bones.size(),
                        0.0f);

                    for (size_t i = 0; i < skeleton->bones.size(); i++)
                    {
                        boneMask->boneNames[i] =
                        {
                            static_cast<int>(i),
                            skeleton->bones[i].name
                        };
                    }
                }
                

            }

            ImGui::EndDragDropTarget();
        }

        return;
    }


    Skeleton* skeleton =
        assets.GetSkeleton(boneMask->skeletonId);


    ImGui::Text("Skeleton : %s", skeleton->name.c_str());

    ImGui::SameLine();

    if (ImGui::SmallButton("Reset Skeleton"))
    {
        boneMask->skeletonId = UUID::Null;
        boneMask->boneNames.clear();
        boneMask->boneMask.clear();
        return;
    }



    ImGui::Separator();

    std::function<void(SkeletonBoneNode*)> DrawBoneNode;


    auto SetChildren =
        [&](auto&& self, SkeletonBoneNode* node, float value) -> void
        {
            if (!node)
                return;

            boneMask->boneMask[node->index] = value;

            for (SkeletonBoneNode* child : node->childNodes)
            {
                self(self, child, value);
            }
        };

    DrawBoneNode =
        [&](SkeletonBoneNode* node)
        {
            if (!node)
                return;

            int boneIndex = node->index;

            ImGuiTreeNodeFlags flags =
                node->childNodes.empty()
                ? ImGuiTreeNodeFlags_Leaf
                : 0;

            bool open =
                ImGui::TreeNodeEx(
                    (void*)node,
                    flags,
                    "%s",
                    node->name.c_str());

            ImGui::SameLine(250);

            bool enabled =
                boneMask->boneMask[boneIndex] > 0.0f;

            if (ImGui::Checkbox(
                ("##enabled" + std::to_string(boneIndex)).c_str(),
                &enabled))
            {
                SetChildren(
                    SetChildren,
                    node,
                    enabled ? 1.0f : 0.0f);
            }

            if (enabled)
            {
                ImGui::SameLine();

                ImGui::SetNextItemWidth(120);

                ImGui::InputFloat(
                    ("##mask" + std::to_string(boneIndex)).c_str(),
                    &boneMask->boneMask[boneIndex],
                    0.0f,
                    0.0f,
                    "%.3f");
            }

            if (open)
            {
                for (SkeletonBoneNode* child : node->childNodes)
                {
                    DrawBoneNode(child);
                }

                ImGui::TreePop();
            }
        };

    DrawBoneNode(skeleton->rootNode);
}


void InspectorPanel::DrawEntityMaterialEditor(const Entity& entityID)
{
    Scene* scene = sceneManager.GetEditorScene();
    Registry& registry = scene->GetRegistry();

    MeshRenderer& mr = registry.meshRenderers.Get(entityID);
    MaterialInstance& inst = mr.inst;

    if (inst.baseMaterial == UUID::Null)
    {
        ImGui::TextDisabled("No Base Material Assigned");
        return;
    }

    const Material* baseMat = assets.GetMaterial(inst.baseMaterial);
    if (!baseMat)
    {
        ImGui::TextDisabled("Base Material not loaded");
        if (ImGui::Button("Load Base Material"))
            assets.LoadMaterial(inst.baseMaterial);
        return;
    }

    ImGui::Separator();
    ImGui::Text("Material");
    ImGui::Spacing();

    if (ImGui::Checkbox("Render", &mr.render))
    {
    }

    auto DrawVec3Override = [&](const char* label,
        const glm::vec3& baseValue,
        std::optional<glm::vec3>& overrideValue)
        {
            glm::vec3 value = overrideValue.value_or(baseValue);

            if (ImGui::ColorEdit3(label, glm::value_ptr(value))) {
                overrideValue = value;
                inst.dirty = true;
            }
            ImGui::SameLine();
            if (overrideValue.has_value())
            {
                if (ImGui::SmallButton(("Reset##" + std::string(label)).c_str())) {
                    overrideValue.reset();
                    inst.dirty = true;
                }
            }

            ImGui::Spacing();
        };

    auto DrawFloatOverride = [&](const char* label,
        const float baseValue,
        std::optional<float>& overrideValue,
        float speed,
        float min,
        float max)
        {
            float value = overrideValue.value_or(baseValue);

            if (ImGui::DragFloat(label, &value, speed, min, max)) {
                overrideValue = value;
                inst.dirty = true;
            }
            ImGui::SameLine();
            if (overrideValue.has_value())
            {
                if (ImGui::SmallButton(("Reset##" + std::string(label)).c_str())) {
                    overrideValue.reset();
                    inst.dirty = true;
                }
            }

            ImGui::Spacing();
        };

    auto DrawTextureOverride = [&](const char* label,
        TextureMapType mapType,
        bool srgb,
        const UUID& baseSlot,
        std::optional<UUID>& overrideSlot,
        bool& useMap)
        {
            UUID current = overrideSlot.value_or(baseSlot);
            std::string texName = "None";

            if (current != UUID::Null)
                texName = assets.GetAssetMetaData(current)->name;

            if (ImGui::Checkbox(("Use##" + std::string(label)).c_str(), &useMap))
                inst.dirty = true;

            ImGui::SameLine();
            ImGui::Text(label);

            ImGui::BeginDisabled(!useMap);

            ImGui::Button(
                (texName + "##" + std::string(label)).c_str(),
                { ImGui::GetContentRegionAvail().x, 28 }
            );

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("TEXTURE_ASSET"))
                {
                    const TextureDragPayload* data =
                        static_cast<const TextureDragPayload*>(payload->Data);

                    if (!assets.getTexture(data->id))
                        assets.RequestTextureLoad_inst(data->id, entityID, mapType, srgb);

                    overrideSlot = data->id;
                    inst.dirty = true;
                }
                ImGui::EndDragDropTarget();
            }

            if (overrideSlot.has_value())
            {
                if (ImGui::SmallButton(("Reset##" + std::string(label)).c_str()))
                {
                    overrideSlot.reset();
                    inst.dirty = true;
                }
            }

            ImGui::EndDisabled();
            ImGui::Spacing();
        };

    DrawVec3Override("Albedo", baseMat->albedo, inst.albedo);
    DrawFloatOverride("Ambient Occlusion", baseMat->ao, inst.ao, 0.01f, 0.0f, 1.0f);
    DrawFloatOverride("Metallic", baseMat->metallic, inst.metallic, 0.01f, 0.0f, 1.0f);
    DrawFloatOverride("Roughness", baseMat->roughness, inst.roughness, 0.01f, 0.0f, 1.0f);
    DrawFloatOverride("Normal Strength", baseMat->normalStrength, inst.normalStrength, 0.1f, -100.0f, 100.0f);
    DrawFloatOverride("Opacity", baseMat->opacity, inst.opacity, 0.01f, 0.0f, 1.0f);

    ImGui::Spacing();
    ImGui::BeginDisabled(true);
    bool transparentDisplay = baseMat->isTransparent;
    ImGui::Checkbox("Transparent", &transparentDisplay);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("(Set on base material)");
    ImGui::Spacing();

    ImGui::Separator();

    ImGui::Separator();

    DrawTextureOverride("Albedo Map", TextureMapType::Albedo, true, baseMat->map_albedo, inst.map_albedo, inst.use_map_albedo);
    DrawTextureOverride("Normal Map", TextureMapType::Normal, false, baseMat->map_normal, inst.map_normal, inst.use_map_normal);
    DrawTextureOverride("AO Map", TextureMapType::AmbientOcclusion, false, baseMat->map_ao, inst.map_ao, inst.use_map_ao);
    DrawTextureOverride("Metallic Map", TextureMapType::Metallic, false, baseMat->map_metallic, inst.map_metallic, inst.use_map_metallic);
    DrawTextureOverride("Roughness Map", TextureMapType::Roughness, false, baseMat->map_roughness, inst.map_roughness, inst.use_map_roughness);
    DrawTextureOverride("Metallic Roughness Map", TextureMapType::MetallicRoughness, false, baseMat->map_metallicRoughness, inst.map_metallicRoughness, inst.use_map_metallicRoughness);
}


// TODO : break down this func into parts
void InspectorPanel::DrawEntityInspector(const Entity& entityID)
{
    if (entityID == UUID::Null) {
        ImGui::Text("No entity selected.");
        return;
    }

    Scene* scene = sceneManager.GetEditorScene();
    Registry& registry = scene->GetRegistry();
    Entity entity = entityID;

    if (!entity) {
        ImGui::Text("No entity selected.");
        return;
    }

    // ---------------- NAME ----------------

    auto& tag = registry.nameTags.Get(entity);
    char buffer[256] = {};
    strcpy_s(buffer, sizeof(buffer), tag.name.c_str());

    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
        if (buffer[0] != '\0')
            tag.name = buffer;
    }
    ImGui::Spacing();

    // ----- TRANSFORM -----

    if (registry.transforms.Has(entity))
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& tr = registry.transforms.Get(entity);

            ImGui::Text("Position");
            ImGui::SameLine();
            if (ImGui::Button("Reset##pos")) {
                tr.localPosition = { 0.0f, 0.0f, 0.0f };
                tr.localDirty = true;
                tr.worldDirty = true;
            }
            if (ImGui::DragFloat3("##Position", glm::value_ptr(tr.localPosition), 0.1f)) {
                tr.localDirty = true;
                tr.worldDirty = true;
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            if (ImGui::Button("Reset##rot")) {
                tr.localRotation = glm::vec3(0.0f);
                tr.localDirty = true;
                tr.worldDirty = true;
            }
            glm::vec3 rotationDeg = glm::degrees(glm::eulerAngles(tr.localRotation));
            if (ImGui::DragFloat3("##Rotation", glm::value_ptr(rotationDeg), 0.5f)) {
                tr.localRotation = glm::radians(rotationDeg);
                tr.localDirty = true;
                tr.worldDirty = true;
            }

            ImGui::Text("Scale");
            ImGui::SameLine();
            if (ImGui::Button("Reset##scale")) {
                tr.localScale = { 1.0f, 1.0f, 1.0f };
                tr.localDirty = true;
                tr.worldDirty = true;
            }
            bool& uniformScale = inspectorState.uniformScale;
            ImGui::Checkbox("Uniform", &uniformScale);
            if (uniformScale) {
                float s = tr.localScale.x;
                if (ImGui::DragFloat("##UniformScale", &s, 0.05f)) {
                    tr.localScale = { s, s, s };
                    tr.localDirty = true;
                    tr.worldDirty = true;
                }
            }
            else {
                if (ImGui::DragFloat3("##Scale", glm::value_ptr(tr.localScale), 0.05f))
                    tr.localDirty = true;
            }
        }
    }

    // ---------------- MESH FILTER ----------------

    if (registry.meshFilters.Has(entityID))
    {
        MeshFilter& mf = registry.meshFilters.Get(entity);

        if (ImGui::CollapsingHeader("Mesh Filter", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BeginGroup();

            std::string submeshName = "None";
            const UUID& meshID = mf.meshID;

            if (!meshID.isNull() && !mf.HasPendingSubmesh()) {
                const AssetMetadata* submeshMetaData = assets.GetAssetMetaData(mf.meshID);
                submeshName = submeshMetaData ? submeshMetaData->name : "Invalid Submesh";
            }

            ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 30 };

            if (ImGui::SmallButton("Reset"))
                mf.meshID = UUID::Null;
            ImGui::SameLine();
            ImGui::Button((submeshName + "##Submesh").c_str(), buttonSize);

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SUBMESH_ASSET")) {
                    const MeshDragPayload* data = (const MeshDragPayload*)payload->Data;
                    UUID droppedID = data->id;
                    if (assets.assetStates[droppedID] == AssetState::Loaded)
                        mf.meshID = droppedID;
                    else {
                        assets.RequestSubmeshLoad(droppedID, entity);
                        mf.RequestSubmesh(droppedID);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndGroup();
        }
    }

    // ---------------- MESH RENDERER ----------------

    if (registry.meshRenderers.Has(entityID))
    {
        MeshRenderer& mr = registry.meshRenderers.Get(entity);

        if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BeginGroup();

            std::string materialName = "None";
            UUID materialID = mr.inst.baseMaterial;

            if (!materialID.isNull()) {
                const AssetMetadata* materialMetaData = assets.GetAssetMetaData(materialID);
                materialName = materialMetaData ? materialMetaData->name : "Invalid Material";
            }

            ImVec2 buttonSize = { ImGui::GetContentRegionAvail().x, 30 };
            ImGui::Button((materialName + "##Material").c_str(), buttonSize);

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PBRMATERIAL_ASSET")) {
                    const MaterialDragPayload* data = (const MaterialDragPayload*)payload->Data;
                    UUID droppedID = data->id;
                    if (assets.GetMaterial(droppedID)) {
                        mr.inst.baseMaterial = droppedID;
                        mr.inst.dirty = true;
                    }
                    else {
                        if (assets.LoadMaterial(droppedID)) {
                            mr.inst.baseMaterial = droppedID;
                            mr.inst.dirty = true;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            DrawEntityMaterialEditor(entityID);
            ImGui::EndGroup();
        }
    }

    // ---------------- LIGHT ----------------

    if (registry.lights.Has(entity))
    {
        auto& light = registry.lights.Get(entity);

        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* lightTypes[] = { "Directional", "Point", "Spotlight" };
            int currentType = static_cast<int>(light.type);
            if (ImGui::Combo("Type", &currentType, lightTypes, IM_ARRAYSIZE(lightTypes)))
                light.type = static_cast<LightType>(currentType);

            ImGui::Spacing();
            ImGui::Text("Color");
            ImGui::SameLine();
            ImGui::ColorEdit3("##LightColor", glm::value_ptr(light.color));
            ImGui::Spacing();
            ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 100000.0f, "%.2f");
            ImGui::Spacing();

            bool isShadowCaster = false;
            if (light.type == LightType::Directional)
                isShadowCaster = scene->GetDirectionalShadowCaster() == entityID;
            else
                isShadowCaster = scene->GetPointShadowCaster() == entityID;

            if (ImGui::Checkbox("Cast Shadows", &isShadowCaster)) {
                light.castShadow = isShadowCaster;
                if (light.type == LightType::Directional)
                    scene->SetDirectionalShadowCaster(isShadowCaster ? entityID : UUID::Null);
                else
                    scene->SetPointShadowCaster(isShadowCaster ? entityID : UUID::Null);
            }

            if (light.type == LightType::Point || light.type == LightType::Spotlight) {
                ImGui::Spacing();
                ImGui::DragFloat("Range", &light.range, 0.1f, 0.1f, 1000.0f, "%.2f");
            }

            if (light.type == LightType::Spotlight) {
                ImGui::Spacing();
                ImGui::DragFloat("Inner Angle", &light.innerAngle, 0.1f, 0.0f, light.outerAngle - 0.1f, "%.1f deg");
                ImGui::DragFloat("Outer Angle", &light.outerAngle, 0.1f, light.innerAngle + 0.1f, 90.0f, "%.1f deg");
            }

            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Remove Light Component"))
                registry.lights.Remove(entity);
            ImGui::Spacing();
        }
    }

    // ---------------- ANIMATION ----------------

    if (registry.animations.Has(entityID))
    {
        AnimationComponent& animComponent = registry.animations.Get(entityID);
        AnimatorController& ctrl = animComponent.animator;

        bool removeAnimationComponent = false;

        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Remove##anim"))
                removeAnimationComponent = true;

            ImGui::Separator();

            if (ImGui::Button("Animator Controller") && onOpenAnimatorEditor)
                onOpenAnimatorEditor(EditorSelection::GetEntity());

            ImGui::Separator();

            if (ImGui::TreeNode("All Clips"))
            {
                for (UUID id : animComponent.animationIDs)
                {
                    Animation* anim = assets.GetAnimation(id);
                    if (!anim) continue;

                    auto& animNames = animComponent.animationNames;
                    auto& nameToID = animComponent.animationNameToID;

                    ImGui::PushID((int)id);

                    char buffer[256];
                    strcpy_s(buffer, animNames[id].c_str());

                    if (ImGui::InputText("Name", buffer, sizeof(buffer),
                        ImGuiInputTextFlags_EnterReturnsTrue))   // commit only on Enter, not every keystroke
                    {
                        std::string oldName = animNames[id];
                        std::string newName = buffer;

                        //ignore if empty or if name is taken by a different clip
                        bool nameConflict = nameToID.count(newName) && nameToID[newName] != id;
                        bool unchanged = (newName == oldName);

                        if (!newName.empty() && !nameConflict && !unchanged)
                        {
                            nameToID.erase(oldName);
                            animNames[id] = newName;
                            nameToID[newName] = id;
                        }
                    }

                    ImGui::SameLine();
                    ImGui::TextDisabled("(%.1f ticks, %.0f tps)",
                        anim->duration,
                        anim->ticksPerSecond);

                    ImGui::PopID();
                }

                if (animComponent.animationIDs.empty())
                    ImGui::TextDisabled("None");

                ImGui::TreePop();
            }
        }

        if (removeAnimationComponent)
            registry.animations.Remove(entityID);
    }

    // ---------------- SKELETON ----------------

    if (registry.skeletons.Has(entityID))
    {
        SkeletonComponent& skeletonComponent =
            registry.skeletons.Get(entityID);

        Skeleton* skeleton =
            assets.GetSkeleton(skeletonComponent.skeletonID);

        if (skeleton)
        {
            if (ImGui::CollapsingHeader("Skeleton", ImGuiTreeNodeFlags_DefaultOpen))
            {
                std::function<void(SkeletonBoneNode*)> DrawBoneNode;

                DrawBoneNode = [&](SkeletonBoneNode* node)
                    {
                        if (!node)
                            return;

                        ImGuiTreeNodeFlags flags =
                            node->childNodes.empty()
                            ? ImGuiTreeNodeFlags_Leaf
                            : 0;

                        if (ImGui::TreeNodeEx(
                            (void*)node,
                            flags,
                            "%s",
                            node->name.c_str()))
                        {
                            for (SkeletonBoneNode* child : node->childNodes)
                                DrawBoneNode(child);

                            ImGui::TreePop();
                        }
                    };

                DrawBoneNode(skeleton->rootNode);
            }
        }
    }

    // ---------------- BONE ATTACHMENT ----------------

    if (registry.boneAttachments.Has(entityID))
    {
        BoneAttachmentComponent& attachment =
            registry.boneAttachments.Get(entityID);

        bool removeBoneAttachment = false;

        if (ImGui::CollapsingHeader("Bone Attachment",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Remove##boneattachment"))
                removeBoneAttachment = true;

            ImGui::Separator();

            ImGui::TextDisabled(
                "Using Parent: %s",
                attachment.skinnedRoot == NullEntity
                ? "No Parent"
                : registry.nameTags.Get(attachment.skinnedRoot).name.c_str()
            );
                
            
            ImGui::Separator();


            static int rootMode = 0; // 0 = Root Parent, 1 = Raw Entity

            ImGui::Text("Skinned Root");
            ImGui::RadioButton("Root Parent", &rootMode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Raw Entity", &rootMode, 1);

            if (rootMode == 0)
            {

                ImGui::TextDisabled(
                    "Using Root Parent (%u)",
                    (uint32_t)attachment.modelRoot
                );
            }
            else
            {
                uint32_t entityValue =
                    (uint32_t)attachment.modelRoot;

                if (ImGui::InputScalar(
                    "Entity ID",
                    ImGuiDataType_U32,
                    &entityValue))
                {
                    attachment.modelRoot =
                        Entity(entityValue);
                }
            }

            ImGui::Separator();

            ImGui::InputInt(
                "Bone Index",
                &attachment.boneIndex
            );

            ImGui::Separator();

            if (ImGui::TreeNode("Offset Matrix"))
            {
                for (int row = 0; row < 4; row++)
                {
                    ImGui::PushID(row);

                    float values[4] =
                    {
                        attachment.offset[row][0],
                        attachment.offset[row][1],
                        attachment.offset[row][2],
                        attachment.offset[row][3]
                    };

                    if (ImGui::InputFloat4("##row", values))
                    {
                        attachment.offset[row][0] = values[0];
                        attachment.offset[row][1] = values[1];
                        attachment.offset[row][2] = values[2];
                        attachment.offset[row][3] = values[3];
                    }

                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }

        if (removeBoneAttachment)
            registry.boneAttachments.Remove(entityID);
    }

    // ---------------- CAMERA ----------------

    if (registry.cameras.Has(entity))
    {
        auto& editorCamera = registry.cameras.Get(entity);

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* projectionTypes[] = { "Perspective", "Orthographic" };
            int currentType = static_cast<int>(editorCamera.projectionType);
            if (ImGui::Combo("Projection", &currentType, projectionTypes, IM_ARRAYSIZE(projectionTypes))) {
                editorCamera.projectionType = static_cast<ProjectionType>(currentType);
                editorCamera.recalculateProjection();
            }

            ImGui::Spacing();

            if (editorCamera.projectionType == ProjectionType::Perspective) {
                if (ImGui::DragFloat("FOV", &editorCamera.fov, 0.1f, 1.0f, 179.0f, "%.1f deg"))
                    editorCamera.recalculateProjection();
                if (ImGui::DragFloat("Aspect Ratio", &editorCamera.aspectRatio, 0.01f, 0.1f, 10.0f))
                    editorCamera.recalculateProjection();
            }

            if (editorCamera.projectionType == ProjectionType::Orthographic) {
                if (ImGui::DragFloat("Ortho Size", &editorCamera.orthoSize, 0.1f, 0.1f, 1000.0f))
                    editorCamera.recalculateProjection();
                if (ImGui::DragFloat("Aspect Ratio", &editorCamera.aspectRatio, 0.01f, 0.1f, 10.0f))
                    editorCamera.recalculateProjection();
            }

            ImGui::Spacing();
            if (ImGui::DragFloat("Near Clip", &editorCamera.nearClip, 0.01f, 0.001f, editorCamera.farClip - 0.01f))
                editorCamera.recalculateProjection();
            if (ImGui::DragFloat("Far Clip", &editorCamera.farClip, 1.0f, editorCamera.nearClip + 0.01f, 100000.0f))
                editorCamera.recalculateProjection();

            ImGui::Spacing();

            ImGui::Separator();
            ImGui::Text("Target");

            // Current target shown in a box
            std::string targetName = "None";

            if (editorCamera.target != NullEntity &&
                registry.nameTags.Has(editorCamera.target))
            {
                targetName =
                    registry.nameTags.Get(editorCamera.target).name;
            }

            ImGui::Button(targetName.c_str(), ImVec2(-1.0f, 0.0f));


            // Clipboard info + Paste button on same line
            std::string copiedName = "None";

            Entity copiedEntity = EditorSelection::GetCopiedEntity();
            if (copiedEntity != NullEntity &&
                registry.nameTags.Has(copiedEntity))
            {
                copiedName =
                    registry.nameTags.Get(copiedEntity).name;
            }

            ImGui::TextDisabled("Clipboard: %s", copiedName.c_str());

            ImGui::SameLine();

            if (ImGui::SmallButton("Paste"))
            {
                editorCamera.target = copiedEntity;
            }


            if (ImGui::SmallButton("Clear"))
            {
                editorCamera.target = NullEntity;
            }

            ImGui::Separator();
            ImGui::Spacing();


            bool isPrimary = scene->GetPrimaryCamera() == entity;
            if (ImGui::Checkbox("Primary Camera", &isPrimary)) {
                if (isPrimary)
                    scene->SetPrimaryCamera(entity);
                else
                    scene->SetPrimaryCamera(NullEntity);
            }

            ImGui::Separator();
            if (ImGui::Button("Remove Camera Component"))
                registry.cameras.Remove(entity);
            ImGui::Spacing();
        }
    }

    // ---------------- RIGIDBODY ----------------

    if (registry.rigidBodies.Has(entity))
    {
        auto& rb = registry.rigidBodies.Get(entity);

        if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // --- Mass ---
            ImGui::Text("Mass");
            ImGui::SameLine();
            if (ImGui::Button("Reset##mass"))
                physSystem.SetMass(entity, 1.0f);

            float mass = rb.mass;
            if (ImGui::DragFloat("##Mass", &mass, 0.1f, 0.01f, 1000.0f))
                physSystem.SetMass(entity, mass);

            ImGui::Spacing();

            // --- Flags ---
            bool useGravity = rb.useGravity;
            bool isKinematic = rb.isKinematic;

            if (ImGui::Checkbox("Use Gravity", &useGravity))
                physSystem.SetGravityEnabled(entity, useGravity);

            if (ImGui::Checkbox("Is Kinematic", &isKinematic))
                physSystem.SetKinematic(entity, isKinematic);

            ImGui::Spacing();

            // --- Damping ---
            ImGui::Text("Damping");

            float linearDamping = rb.linearDamping;
            float angularDamping = rb.angularDamping;

            ImGui::Text("Linear");
            ImGui::SameLine();
            if (ImGui::Button("Reset##lindamp"))
                physSystem.SetLinearDamping(entity, 0.1f);
            if (ImGui::DragFloat("##LinearDamping", &linearDamping, 0.01f, 0.0f, 10.0f))
                physSystem.SetLinearDamping(entity, linearDamping);

            ImGui::Text("Angular");
            ImGui::SameLine();
            if (ImGui::Button("Reset##angdamp"))
                physSystem.SetAngularDamping(entity, 0.05f);
            if (ImGui::DragFloat("##AngularDamping", &angularDamping, 0.01f, 0.0f, 10.0f))
                physSystem.SetAngularDamping(entity, angularDamping);

            ImGui::Spacing();

            // --- Linear Axis Locks ---
            ImGui::Text("Freeze Position");

            bool lx = rb.lockLinearX;
            bool ly = rb.lockLinearY;
            bool lz = rb.lockLinearZ;

            bool linearChanged = false;
            linearChanged |= ImGui::Checkbox("X##linX", &lx); ImGui::SameLine();
            linearChanged |= ImGui::Checkbox("Y##linY", &ly); ImGui::SameLine();
            linearChanged |= ImGui::Checkbox("Z##linZ", &lz);

            if (linearChanged)
                physSystem.SetLinearLock(entity, lx, ly, lz);

            // --- Angular Axis Locks ---
            ImGui::Text("Freeze Rotation");

            bool ax = rb.lockAngularX;
            bool ay = rb.lockAngularY;
            bool az = rb.lockAngularZ;

            bool angularChanged = false;
            angularChanged |= ImGui::Checkbox("X##angX", &ax); ImGui::SameLine();
            angularChanged |= ImGui::Checkbox("Y##angY", &ay); ImGui::SameLine();
            angularChanged |= ImGui::Checkbox("Z##angZ", &az);

            if (angularChanged)
                physSystem.SetAngularLock(entity, ax, ay, az);

            ImGui::Spacing();

            // --- Remove ---
            if (ImGui::Button("Remove Rigidbody"))
                registry.rigidBodies.Remove(entity);
        }
    }

    // ---------------- COLLIDER ----------------

    if (registry.colliders.Has(entity))
    {
        auto& col = registry.colliders.Get(entity);

        if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (size_t i = 0; i < col.shapes.size(); i++)
            {
                auto& shape = col.shapes[i];
                ImGui::PushID((int)i);

                if (ImGui::TreeNode("Shape"))
                {
                    const char* colliderTypes[] = { "Box", "Sphere", "Capsule" };
                    int currentType = (int)shape.type;

                    if (ImGui::Combo("Type", &currentType, colliderTypes, IM_ARRAYSIZE(colliderTypes)))
                    {
                        shape.type = (ColliderShape::Type)currentType;
                        shape.dirty = true;
                    }

                    if (shape.type == ColliderShape::Type::Box)
                        if (ImGui::DragFloat3("Size", glm::value_ptr(shape.size), 0.1f))
                            shape.dirty = true;

                    if (shape.type == ColliderShape::Type::Sphere)
                        if (ImGui::DragFloat("Radius", &shape.radius, 0.05f))
                            shape.dirty = true;

                    if (shape.type == ColliderShape::Type::Capsule)
                    {
                        if (ImGui::DragFloat("Radius", &shape.radius, 0.05f))
                            shape.dirty = true;
                        if (ImGui::DragFloat("Height", &shape.height, 0.05f))
                            shape.dirty = true;
                    }

                    if (ImGui::Checkbox("Is Trigger", &shape.isTrigger))
                        shape.dirty = true;

                    ImGui::Spacing();

                    if (ImGui::Button("Delete Shape"))
                    {
                        // DeleteColliderShape is still a direct call — it targets a
                        // specific shape slot by index, which the hook system has no
                        // way to express. Fine to call immediately from the editor.
                        physSystem.DeleteColliderShape(entity, col, i);
                        ImGui::TreePop();
                        ImGui::PopID();
                        break;
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            ImGui::Spacing();

            // Adding individual shapes to an existing collider is also a direct
            // call — the hook only fires when the whole ColliderComponent is
            // added/removed, not when a shape is pushed into an existing one.
            if (ImGui::Button("Add Box"))
                physSystem.AddCollider(entity, col, ColliderShape::Type::Box);
            ImGui::SameLine();
            if (ImGui::Button("Add Sphere"))
                physSystem.AddCollider(entity, col, ColliderShape::Type::Sphere);
            ImGui::SameLine();
            if (ImGui::Button("Add Capsule"))
                physSystem.AddCollider(entity, col, ColliderShape::Type::Capsule);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Delete Collider Component"))
            {
                // Same as rigidbody — removing from registry fires onRemove,
                // which queues the full collider teardown. No manual phys call needed.
                registry.colliders.Remove(entity);
            }
        }
    }
    // ---------------- CONTROLLER ----------------

    if (registry.controllers.Has(entity))
    {
        auto& controller = registry.controllers.Get(entity);

        if (ImGui::CollapsingHeader("Controller", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Active", &controller.active);
            ImGui::Spacing();

            const char* controllerTypes[] = { "None", "Player", "AI" };
            int currentType = static_cast<int>(controller.type);
            if (ImGui::Combo("Controller Type", &currentType, controllerTypes, IM_ARRAYSIZE(controllerTypes)))
                controller.type = static_cast<ControllerType>(currentType);

            ImGui::Separator();
            ImGui::Text("Key Bindings");

            ImGui::Text("Forward");   ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.moveForward), ImVec2(120, 0));
            ImGui::Text("Backward");  ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.moveBackward), ImVec2(120, 0));
            ImGui::Text("Left");      ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.moveLeft), ImVec2(120, 0));
            ImGui::Text("Right");     ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.moveRight), ImVec2(120, 0));

            ImGui::Separator();

            ImGui::Text("Jump");      ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.jump), ImVec2(120, 0));
            ImGui::Text("Interact");  ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.interact), ImVec2(120, 0));
            ImGui::Text("Pause");     ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.pause), ImVec2(120, 0));
            ImGui::Text("Sprint");    ImGui::SameLine(); ImGui::Button(SDL_GetKeyName(controller.keybinds.sprint), ImVec2(120, 0));

            ImGui::Separator();

            const char* attackMouseName = "Unknown";
            switch (controller.keybinds.attack) {
            case SDL_BUTTON_LEFT:   attackMouseName = "Mouse Left";   break;
            case SDL_BUTTON_RIGHT:  attackMouseName = "Mouse Right";  break;
            case SDL_BUTTON_MIDDLE: attackMouseName = "Mouse Middle"; break;
            }
            ImGui::Text("Attack"); ImGui::SameLine(); ImGui::Button(attackMouseName, ImVec2(120, 0));

            ImGui::Separator();
            ImGui::Text("Runtime State");

            ImGui::DragFloat("Move X", &controller.moveX, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Move Y", &controller.moveY, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Look X", &controller.lookX, 0.01f);
            ImGui::DragFloat("Look Y", &controller.lookY, 0.01f);
            ImGui::Checkbox("Sprint Held", &controller.sprintHeld);
            ImGui::Checkbox("Jump Pressed", &controller.jumpPressed);
            ImGui::Checkbox("Attack Pressed", &controller.attackPressed);
            ImGui::Checkbox("Interact Pressed", &controller.interactPressed);
            ImGui::Checkbox("Pause Pressed", &controller.pausePressed);

            ImGui::Spacing();
            if (ImGui::Button("Reset Keybinds"))
                controller.keybinds = GameKeys{};
            ImGui::Separator();
            if (ImGui::Button("Remove Controller Component"))
                registry.controllers.Remove(entity);
            ImGui::Spacing();
        }
    }

    // ---------------- MOVEMENT ----------------

    if (registry.movements.Has(entity))
    {
        auto& movement = registry.movements.Get(entity);

        if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Movement Input");
            ImGui::DragFloat2("Move Input", &movement.moveInput.x, 0.01f, -1.0f, 1.0f);
            ImGui::Checkbox("Jump Requested", &movement.jumpRequested);
            ImGui::Checkbox("Sprinting", &movement.sprinting);

            ImGui::Separator();
            ImGui::Text("Movement Settings");

            ImGui::DragFloat("Walk Speed", &movement.walkSpeed, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Sprint Multiplier", &movement.sprintMultiplier, 0.05f, 1.0f, 10.0f);
            ImGui::DragFloat("Jump Force", &movement.jumpForce, 0.1f, 0.0f, 100.0f);

            ImGui::Separator();
            ImGui::Text("Input Magnitude: %.2f", glm::length(movement.moveInput));
            ImGui::Separator();

            if (ImGui::Button("Remove Movement Component"))
                registry.movements.Remove(entity);
            ImGui::Spacing();
        }
    }

    // ---------------- SCRIPT ----------------

    if (registry.scripts.Has(entity))
    {
        DrawScriptComponent(entity, registry);
    }



    // Invisible full-width dummy to catch right-clicks in empty space
    ImGui::InvisibleButton("##inspector_ctx_area",
        ImVec2(ImGui::GetContentRegionAvail().x,
            (std::max)(ImGui::GetContentRegionAvail().y, 8.0f)));

    if (ImGui::BeginPopupContextItem("##add_component_popup",
        ImGuiPopupFlags_MouseButtonRight))
    {
        DrawAddComponentMenu(entityID, entity, registry, scene);
        ImGui::EndPopup();
    }

    // Also catch right-click on the window background itself
    if (ImGui::BeginPopupContextWindow("##add_component_window",
        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        DrawAddComponentMenu(entityID, entity, registry, scene);
        ImGui::EndPopup();
    }
}
void InspectorPanel::DrawSaveOrCancelPopup()
{
    if (EditorSelection::GetAsset().first == UUID::Null) return;

    if (s_OpenSaveOrCancelPopup)
    {
        ImGui::OpenPopup("Unsaved Material");
        s_OpenSaveOrCancelPopup = false;
    }

    if (ImGui::BeginPopupModal(
        "Unsaved Material",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("You have unsaved changes.");
        ImGui::Text("Do you want to save before closing?");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            const UUID matID = EditorSelection::GetAsset().first;
            Material* mat = assets.GetMaterial(matID);
            mat->localDirty = false;

            assets.SaveMaterial(matID);

            isAssetSaved = true;
            EditorSelection::ClearAssetSelection();

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void InspectorPanel::HandleAssetEditorClear()
{
    //if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    //{
    //    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
    //        !s_IsHovered)
    //    {
    //        if (!ImGui::IsAnyItemHovered())
    //        {
    //            if (!isAssetSaved) {
    //                s_OpenSaveOrCancelPopup = true;
    //            }
    //            else {
    //                EditorSelection::ClearAssetSelection();
    //            }
    //        }
    //    }
    //}
}

void InspectorPanel::DrawAddComponentMenu(
    const Entity& entityID,
    const Entity& entity,
    Registry& registry,
    Scene* scene)
{
    if (ImGui::BeginMenu("Add Component"))
    {
        if (!registry.transforms.Has(entity))
            if (ImGui::MenuItem("Transform"))
                registry.transforms.Add(entity);

        if (!registry.meshFilters.Has(entityID))
            if (ImGui::MenuItem("Mesh Filter"))
                registry.meshFilters.Add(entity);

        if (!registry.meshRenderers.Has(entityID))
            if (ImGui::MenuItem("Mesh Renderer"))
                registry.meshRenderers.Add(entityID);

        if (!registry.lights.Has(entity))
            if (ImGui::MenuItem("Light"))
                registry.lights.Add(entity);

        if (!registry.cameras.Has(entity))
            if (ImGui::MenuItem("Camera"))
            {
                auto& cam = registry.cameras.Add(entity);
                cam.recalculateProjection();
            }

        if (!registry.boneAttachments.Has(entity))
            if (ImGui::MenuItem("Bone Attachment"))
            {
               registry.boneAttachments.Add(entity);
            }

        if (!registry.rigidBodies.Has(entity))
            if (ImGui::MenuItem("Rigidbody"))
            {
                if (registry.transforms.Has(entityID)) {
                    registry.rigidBodies.Add(entityID);
                }
                // silently ignored if no Transform — 
                // optionally: ImGui::SetTooltip(...)
            }

        if (!registry.colliders.Has(entityID))
            if (ImGui::MenuItem("Collider"))
            {
                if (registry.transforms.Has(entityID))
                    registry.colliders.Add(entityID);
            }

        if (!registry.controllers.Has(entity))
            if (ImGui::MenuItem("Controller"))
            {
                auto& controller = registry.controllers.Add(entity);
                controller.type = ControllerType::Player;
                controller.active = true;
            }

        if (!registry.movements.Has(entity))
            if (ImGui::MenuItem("Movement"))
                registry.movements.Add(entity);

        if (!registry.scripts.Has(entity))
            if (ImGui::MenuItem("Script"))
                registry.scripts.Add(entity);

        ImGui::EndMenu();
    }
}

void InspectorPanel::DrawScriptComponent(const Entity& entityID, Registry& registry)
{
    if (!ImGui::CollapsingHeader("Scripts", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ScriptComponent& sc = registry.scripts.Get(entityID);

    // --- List all attached scripts ---
    for (int i = 0; i < (int)sc.scriptNames.size(); i++)
    {
        ImGui::PushID(i);

        ImGui::BeginGroup();

        // Script name display
        ImGui::Text("%s", sc.scriptNames[i].c_str());

        // Source file tooltip on hover
        const ScriptMetadata* meta = scriptSystem.GetLibrary().FindMetadata(sc.scriptNames[i]);
        if (meta && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", meta->sourceFile.c_str());

        // Remove button inline
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
        if (ImGui::SmallButton("Remove"))
        {
            sc.scriptNames.erase(sc.scriptNames.begin() + i);

            if (i < (int)sc.scripts.size())
                sc.scripts.erase(sc.scripts.begin() + i);

            ImGui::EndGroup();
            ImGui::PopID();
            break;
        }

        ImGui::EndGroup();
        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::Spacing();

    // --- Drop zone to add scripts by dragging from asset panel ---
    ImGui::Button("Drop Script Here##scriptdrop",
        { ImGui::GetContentRegionAvail().x, 30 });

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("SCRIPT_ASSET"))
        {
            const ScriptDragPayload* data =
                (const ScriptDragPayload*)payload->Data;

            // Don't add duplicates
            std::string name = data->name;
            bool alreadyAttached = std::find(
                sc.scriptNames.begin(),
                sc.scriptNames.end(),
                name) != sc.scriptNames.end();

            if (!alreadyAttached)
                sc.scriptNames.push_back(name);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // --- Remove entire component ---
    if (ImGui::Button("Remove Script Component"))
    {
        sc.scriptNames.clear();
        sc.scripts.clear();
        registry.scripts.Remove(entityID);
    }

    ImGui::Spacing();
}