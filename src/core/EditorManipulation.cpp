#include "EditorManipulation.h"
#include "EditorSelection.h"


using namespace Lengine;



void EditorManipulator::CheckForHoveredEntity()
{
    hoveredEntity = nullptr;

    ImVec2 mouse = viewportPanel.getMousePosInViewport();
    ImVec2 vpSize = viewportPanel.GetViewportSize();

    glm::mat4 view = editorCamera.getViewMatrix();
    glm::mat4 projection = editorCamera.getProjectionMatrix();

    glm::vec3 rayDir = ComputeRayDirection(
        mouse.x,
        mouse.y,
        vpSize.x,
        vpSize.y,
        view,
        projection
    );

    glm::vec3 rayOrigin = editorCamera.getCameraPosition();

    const auto& entities = sceneManager.getActiveScene()->getEntities();
    float closest = 999999.0f;

    Scene* activeScene = sceneManager.getActiveScene();

    for (auto& e : entities)
    {
        if (!activeScene->MeshFilters().Has(e->getID())) continue;

        MeshFilter& mf = activeScene->MeshFilters().Get(e->getID());
        TransformComponent& tf = activeScene->Transforms().Get(e->getID());

        glm::vec3 pos = tf.GetWorldPosition();
        glm::vec3 scale = tf.GetWorldScale();

        if (!mf.submeshID)
        {
            float radius = 1.0f;

            if (rayIntersectsSphere(rayOrigin, rayDir, pos, radius))
            {
                float dist = glm::distance(rayOrigin, pos);

                if (dist < closest)
                {
                    closest = dist;
                    hoveredEntity = e.get();
                }
            }
        }
        else
        {
            Submesh* submesh = assetManager.GetSubmesh(mf.submeshID);

            if (!submesh) continue;

            glm::vec3 localCenter = submesh->getLocalCenter();
            float localRadius = submesh->getBoundingRadius();

            glm::vec3 worldCenter = pos + localCenter * scale;

            float maxScale = glm::max(scale.x, glm::max(scale.y, scale.z));
            float worldRadius = localRadius * maxScale;

            if (rayIntersectsSphere(rayOrigin, rayDir, worldCenter, worldRadius))
            {
                float dist = glm::distance(rayOrigin, worldCenter);

                if (dist < closest)
                {
                    closest = dist;
                    hoveredEntity = e.get();
                }
            }
        }
    }
}

void EditorManipulator::SelectHoveredEntity()
{
    CheckForHoveredEntity();

    if (!hoveredEntity)
    {
        selectedEntity = nullptr;
        EditorSelection::ClearEntitySelection();
        return;
    }

    selectedEntity = hoveredEntity;

    EditorSelection::SetEntity(selectedEntity->getID());
}

void EditorManipulator::HandleKeyboardShortcuts(const SDL_Keycode& key)
{
    if (!selectedEntity) return;

    Scene* scene = sceneManager.getActiveScene();

    TransformComponent& tf =
        scene->Transforms().Get(selectedEntity->getID());

    float speed = movementSpeed *
        (inputManager.isKeyDown(EditorKeys::FastMove) ? 5.0f : 1.0f);

    if (inputManager.isKeyPressed(key))
    {
        switch (key)
        {
        case EditorKeys::Delete:

            scene->RemoveEntity(selectedEntity->getID());

            selectedEntity = nullptr;

            EditorSelection::ClearEntitySelection();

            break;
        }
    }
}