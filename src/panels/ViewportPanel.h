#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#define IMGUI_ENABLE_DOCKING
#define IMGUI_ENABLE_DOCKING_EXTENSION
#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "external/ImGuizmo.h"

#include "EditorSelection.h"

#include "graphics/camera/Camera3d.h"
#include "graphics/frameBuffers/Framebuffer.h"


#include "utils/fps.h"
#include "utils/imGuiScreens.h"

#include "scene/SceneManager.h"

#include "platform/Window.h"



namespace Lengine {



    enum class ViewportMode {
        first = 0,
        second,
        third,
        count 
    };

    class ViewportPanel {
    public:
        
        ViewportPanel(Window& window, Camera3d& camera_, SceneManager& scene_) :
            window(window),
            editorCamera(camera_),
            sceneManager(scene_)
        {}

       
        void OnImGuiRender(const uint32_t finalImage);
        void RenderFullscreen(const uint32_t finalImage);

        void ClearFrame(const glm::vec4& clearColor);

        // Check if the viewport has resized
        bool IsViewportFocused() const { return m_Focused; }
        bool IsViewportHovered() const { return m_Hovered; }

        ImVec2 GetViewportSize() const { return m_ViewportSize; }
        ImVec2 GetViewportPos() const { return m_ViewportPos; }

        ImVec2 getMousePosInViewport() const { return mouseInViewport; }
        ImVec2 getMousePosInImage() const { return mouseInImage; }

        bool fixCamera = false;
        bool viewportFullscreen = true;

    public:
        void DrawTransformGizmo();
        
    private:
        Window& window;
        Camera3d& editorCamera;
        SceneManager& sceneManager;

        float fullscreenAspectRatio = 16.0f / 9.0f;


        float offsetValueX = 0.14f;
        float offsetValueY = -0.1f;
        ImVec2 m_ViewportSize = { 1920, 1080 };
        ImVec2 m_ViewportPos;
        ImVec2 m_LastViewportSize = { -1, -1 };

        ImVec2 imagePos;
        ImVec2 mouseInViewport;
        ImVec2 mouseInImage;


        bool m_Focused = false; 
        bool m_Hovered = false;  

        ViewportMode mode = ViewportMode::first;
        
        void handleMouseInViewport();

    };

}
