#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#define IMGUI_ENABLE_DOCKING
#define IMGUI_ENABLE_DOCKING_EXTENSION
#include "imgui.h"
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

// Editor
#include "Editor/core/EditorSelection.h"
#include "Editor/external/ImGuizmo.h"


// Engine
#include "graphics/camera/Camera3d.h"
#include "graphics/frameBuffers/Framebuffer.h"
#include "utils/fps.h"
#include "utils/imGuiScreens.h"
#include "scene/SceneManager.h"
#include "platform/Window.h"
#include "input/InputRouter.h"
#include "input/InputContext.h"







namespace Lengine {
    enum class ViewportType { READ_ONLY, INTERACT };

    enum class ViewportMode {
        first = 0,
        second,
        third,
        count 
    };

    class ViewportPanel {
    public:
        
        ViewportPanel(
            const std::string name,
            const ViewportType type,
            Window& window,
            Camera3d& camera_,
            SceneManager& scene_,
            InputRouter& router_          
        ) :
            window(window),
            editorCamera(camera_),
            sceneManager(scene_),
            inputRouter(router_),        
            viewportName(name),
            type(type)
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
        bool viewportFullscreen = false;

    public:
        void DrawTransformGizmo();
        
    private:
        std::string viewportName;
        ViewportType type = ViewportType::READ_ONLY;
        Window& window;
        Camera3d& editorCamera;
        SceneManager& sceneManager;
        InputRouter& inputRouter;

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
