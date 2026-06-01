#pragma once

#include <SDL2/SDL.h>

// Engine 
#include "input/InputRouter.h"
#include "input/KeyBindings.h"
#include "graphics/camera/Camera3d.h"


namespace Lengine {

    class EditorCameraHandler : public IInputHandler
    {
    public:
        EditorCameraHandler(Camera3d& camera, InputRouter& router)
            : camera(camera)
            , router(router)
        {}

        void onUpdate(float dt, InputManager& input) override
        {
            if (camera.isFixed) return;

            float speed = baseSpeed;
            if (input.isKeyDown(EditorKeys::FastMove))
                speed *= fastMultiplier;

            glm::vec3 dir{ 0.0f };

            if (camera.controlMode == CameraControlMode::first)
            {
                // First-person mode: always move with WASD
                if (input.isKeyDown(EditorKeys::MovePosZ)) dir += camera.getForward();
                if (input.isKeyDown(EditorKeys::MoveNegZ)) dir -= camera.getForward();
                if (input.isKeyDown(EditorKeys::MovePosX)) dir += camera.getRight();
                if (input.isKeyDown(EditorKeys::MoveNegX)) dir -= camera.getRight();
                if (input.isKeyDown(EditorKeys::MovePosY)) dir += glm::vec3(0, 1, 0);
                if (input.isKeyDown(EditorKeys::MoveNegY)) dir -= glm::vec3(0, 1, 0);
            }
            else if (camera.controlMode == CameraControlMode::second)
            {
                // Second-person mode: only move while RMB is held
                if (input.isMouseButtonDown(SDL_BUTTON_RIGHT))
                {
                    if (input.isKeyDown(EditorKeys::MovePosZ)) dir += camera.getForward();
                    if (input.isKeyDown(EditorKeys::MoveNegZ)) dir -= camera.getForward();
                    if (input.isKeyDown(EditorKeys::MovePosX)) dir += camera.getRight();
                    if (input.isKeyDown(EditorKeys::MoveNegX)) dir -= camera.getRight();
                    if (input.isKeyDown(EditorKeys::MovePosY)) dir += glm::vec3(0, 1, 0);
                    if (input.isKeyDown(EditorKeys::MoveNegY)) dir -= glm::vec3(0, 1, 0);
                }
            }

            if (input.isKeyDown(SDLK_c)) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_ENABLE);
                router.setContext(InputContext::UI);

                camera.isFixed = true;
            }

            if (glm::length(dir) > 0.001f)
                camera.move(glm::normalize(dir) * speed * dt);
        }

        void onEvent(const SDL_Event& event, InputManager& input) override
        {
            if (camera.isFixed) return;

            if (event.type == SDL_MOUSEMOTION
                && camera.controlMode == CameraControlMode::first)
            {
                float xrel = static_cast<float>(event.motion.xrel);
                float yrel = static_cast<float>(event.motion.yrel);
                camera.rotate(xrel * mouseSensitivity, yrel * mouseSensitivity);
            }

            if (event.type == SDL_MOUSEMOTION
                && camera.controlMode == CameraControlMode::second
                && input.isMouseButtonDown(SDL_BUTTON_RIGHT))
            {
                float xrel = static_cast<float>(event.motion.xrel);
                float yrel = static_cast<float>(event.motion.yrel);
                camera.rotate(xrel * mouseSensitivity, yrel * mouseSensitivity);
            }

            if (event.type == SDL_MOUSEWHEEL)
            {
                camera.zoom(static_cast<float>(event.wheel.y) * scrollSpeed);
            }
        }

        float baseSpeed = 5.0f;
        float fastMultiplier = 4.0f;
        float mouseSensitivity = 0.15f;
        float scrollSpeed = 0.5f;

    private:
        Camera3d& camera;
        InputRouter& router;
    };

} // namespace Lengine
