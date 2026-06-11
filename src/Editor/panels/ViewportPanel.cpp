#include "ViewportPanel.h"
#include <iostream>

using namespace Lengine;


void ViewportPanel::ClearFrame(const glm::vec4& clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ViewportPanel::OnImGuiRender(const uint32_t finalImage)
{

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(viewportName.c_str());


    if (type == ViewportType::READ_ONLY)
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();

        m_ViewportSize = { avail.x, avail.y };
        m_ViewportPos = ImGui::GetWindowPos();

        GLuint texID = finalImage;

        ImGui::Image(
            (void*)(intptr_t)texID,
            ImVec2(m_ViewportSize.x, m_ViewportSize.y),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }


    ImGui::Separator();

    const char* btnLabel = editorCamera.isFixed ? "Fix Camera: OFF" : "Fix Camera: ON";

    if (ImGui::Button(btnLabel))
    {
        editorCamera.isFixed = !editorCamera.isFixed;

        if (!editorCamera.isFixed)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            SDL_ShowCursor(SDL_DISABLE);
            inputRouter.setContext(InputContext::EditorCamera);
        }
        else
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_ShowCursor(SDL_ENABLE);
            inputRouter.setContext(InputContext::UI);
        }
    }


    ImGui::SameLine();

    if (ImGui::Button("Fullscreen Mode"))
        viewportFullscreen = true;

    ImGui::SameLine();

    static const char* viewportModeLabels[] = {
        "First",
        "Second",
        "Third"
    };

    ViewportMode currentMode = mode;
    int currentModeIndex = static_cast<int>(currentMode);

    ImGui::PushItemWidth(140.0f);

    if (ImGui::Combo(
        "Viewport Mode",
        &currentModeIndex,
        viewportModeLabels,
        static_cast<int>(ViewportMode::count)))
    {
        editorCamera.controlMode =
            static_cast<CameraControlMode>(currentModeIndex);

        mode = static_cast<ViewportMode>(currentModeIndex);
    }

    ImGui::PopItemWidth();

    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();

    m_ViewportSize = { avail.x, avail.y };
    m_ViewportPos = ImGui::GetWindowPos();

    if (m_ViewportSize.x > 0 && m_ViewportSize.y > 0)
    {
        if (m_ViewportSize.x != m_LastViewportSize.x ||
            m_ViewportSize.y != m_LastViewportSize.y)
        {
            m_LastViewportSize = m_ViewportSize;

            editorCamera.setAspectRatio(
                m_ViewportSize.x / m_ViewportSize.y);
        }
    }

    GLuint texID = finalImage;

    ImGui::Image(
        (void*)(intptr_t)texID,
        ImVec2(m_ViewportSize.x, m_ViewportSize.y),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    m_Focused = ImGui::IsItemFocused();
    m_Hovered = ImGui::IsItemHovered();

    imagePos = ImGui::GetItemRectMin();

    ImVec2 mousePos = ImGui::GetMousePos();

    mouseInViewport = {
        mousePos.x - imagePos.x,
        mousePos.y - imagePos.y
    };

    DrawTransformGizmo();

    ImGui::End();
    ImGui::PopStyleVar();
}



void ViewportPanel::RenderFullscreen(const uint32_t finalImage)
{

    editorCamera.setAspectRatio(fullscreenAspectRatio);

    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("ViewportFullscreen", nullptr, flags);

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        viewportFullscreen = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_ShowCursor(SDL_ENABLE);
        inputRouter.setContext(InputContext::UI);
    }

    GLuint texID = finalImage;

    ImVec2 avail = ImGui::GetContentRegionAvail();

    float availAspect = avail.x / avail.y;

    ImVec2 imageSize;
    if (availAspect > fullscreenAspectRatio)
    {
        imageSize.y = avail.y;
        imageSize.x = avail.y * fullscreenAspectRatio;
    }
    else
    {
        imageSize.x = avail.x;
        imageSize.y = avail.x / fullscreenAspectRatio;
    }

    ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(
        cursor.x + (avail.x - imageSize.x) * 0.5f,
        cursor.y + (avail.y - imageSize.y) * 0.5f
    ));

    ImGui::Image(
        (void*)(intptr_t)texID,
        imageSize,
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    ImGui::End();
    ImGui::PopStyleVar();
}

void ViewportPanel::DrawTransformGizmo()
{
    if (!EditorSelection::GetEntity() || !sceneManager.GetEditorScene())
        return;

    Entity selectedEntity = EditorSelection::GetEntity();

    auto* scene = sceneManager.GetEditorScene();

    if (!scene->GetRegistry().transforms.Has(selectedEntity))
        return;

    TransformComponent& transform = scene->GetRegistry().transforms.Get(selectedEntity);

    // Camera
    glm::mat4 view = editorCamera.getViewMatrix();
    glm::mat4 projection = editorCamera.getProjectionMatrix();

    // --- Configure gizmo ---
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(
        imagePos.x,
        imagePos.y,
        m_ViewportSize.x,
        m_ViewportSize.y
    );

    static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    ImGuiIO& io = ImGui::GetIO();

    if (editorCamera.isFixed && !io.WantCaptureKeyboard)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_T))
            operation = ImGuizmo::TRANSLATE;

        if (ImGui::IsKeyPressed(ImGuiKey_E))
            operation = ImGuizmo::ROTATE;

        if (ImGui::IsKeyPressed(ImGuiKey_R))
            operation = ImGuizmo::SCALE;
    }

    // ---- Use world matrix ----
    glm::mat4 transformMatrix = transform.worldMatrix;

    // ---- Draw gizmo ----
    ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(projection),
        operation,
        ImGuizmo::WORLD,
        glm::value_ptr(transformMatrix)
    );

    float size = 128.0f;

    ImGuizmo::ViewManipulate(
        glm::value_ptr(view),
        10.0f,
        ImVec2(
            ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - size - 10,
            ImGui::GetWindowPos().y + 10
        ),
        ImVec2(size, size),
        0x10101010
    );

    if (ImGuizmo::IsUsing())
    {
        // transformMatrix is a world matrix from the gizmo
        // We need to convert it to local space

        glm::mat4 parentWorld(1.0f); // identity = no parent (world space = local space)

        if (scene->GetRegistry().hierarchies.Has(selectedEntity))
        {
            auto& h = scene->GetRegistry().hierarchies.Get(selectedEntity);
            if (h.parent != NullEntity && scene->GetRegistry().transforms.Has(h.parent))
            {
                parentWorld = scene->GetRegistry().transforms.Get(h.parent).worldMatrix;
            }
        }

        // Convert gizmo world matrix → local matrix
        glm::mat4 localMatrix = glm::inverse(parentWorld) * transformMatrix;

        // Decompose local matrix
        glm::vec3 translation = glm::vec3(localMatrix[3]);

        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(localMatrix[0]));
        scale.y = glm::length(glm::vec3(localMatrix[1]));
        scale.z = glm::length(glm::vec3(localMatrix[2]));

        glm::mat3 rotMatrix;
        rotMatrix[0] = glm::vec3(localMatrix[0]) / scale.x;
        rotMatrix[1] = glm::vec3(localMatrix[1]) / scale.y;
        rotMatrix[2] = glm::vec3(localMatrix[2]) / scale.z;

        glm::quat rotation = glm::normalize(glm::quat_cast(rotMatrix));

        // Apply local values
        transform.localPosition = translation;
        transform.localScale = scale;
        transform.localRotation = rotation;

        transform.localDirty = true;
        transform.worldDirty = true;
    }
}

void ViewportPanel::handleMouseInViewport() {

}
