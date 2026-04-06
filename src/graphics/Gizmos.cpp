#include "Gizmos.h"

using namespace Lengine;

void GizmoRenderer::initGizmoGrid() {
    gizmoGrid = assetManager.GetSubmesh(SubmeshID::Plane);
    gizmoGridShader = *assetManager.getShader("Gizmo Grid");


}


void GizmoRenderer::drawGizmoGrid(
    const glm::mat4& view,
    const glm::mat4 proj,
    const glm::vec3 pos
){
    gizmoGridShader.use();
    gizmoGridShader.setMat4("view", view);
    gizmoGridShader.setMat4("projection", proj);
    gizmoGridShader.setVec3("cameraPos", pos);
    if(gizmoGrid) gizmoGrid->draw();
    gizmoGridShader.unuse();

}

