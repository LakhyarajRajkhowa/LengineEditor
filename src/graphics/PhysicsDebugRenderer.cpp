#include "PhysicsDebugRenderer.h"

std::vector<DebugDraw::DebugLine> DebugDraw::lines;

void DebugDraw::Init() {
    shader = std::make_shared<GLSLProgram>();
    shader->compileShaders(Paths::Shaders + "physicsDebug.vert", Paths::Shaders + "physicsDebug.frag");
    shader->linkShaders();

    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);

    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);

    // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLine), (void*)0);

    // colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLine), (void*)offsetof(DebugLine, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
void DebugDraw::Line(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
{
    lines.push_back({ a,b,color });
}

void DebugDraw::WireCube(const glm::mat4& transform,
    const glm::vec3& size,
    const glm::vec3& color)
{
    glm::vec3 h = size * 0.5f;

    glm::vec3 v[8] =
    {
        {-h.x,-h.y,-h.z},
        { h.x,-h.y,-h.z},
        { h.x, h.y,-h.z},
        {-h.x, h.y,-h.z},

        {-h.x,-h.y, h.z},
        { h.x,-h.y, h.z},
        { h.x, h.y, h.z},
        {-h.x, h.y, h.z}
    };

    for (int i = 0; i < 8; i++)
        v[i] = glm::vec3(transform * glm::vec4(v[i], 1.0f));

    Line(v[0], v[1], color);
    Line(v[1], v[2], color);
    Line(v[2], v[3], color);
    Line(v[3], v[0], color);

    Line(v[4], v[5], color);
    Line(v[5], v[6], color);
    Line(v[6], v[7], color);
    Line(v[7], v[4], color);

    Line(v[0], v[4], color);
    Line(v[1], v[5], color);
    Line(v[2], v[6], color);
    Line(v[3], v[7], color);
}

