#include "PhysicsDebugRenderer.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Lengine;

// ---------------------------------------------------------------------------
PhysicsDebugRenderer::PhysicsDebugRenderer(PhysicsSystem& physSystem)
    : physSystem(physSystem)
{}

PhysicsDebugRenderer::~PhysicsDebugRenderer()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

// ---------------------------------------------------------------------------
void PhysicsDebugRenderer::Init()
{
    shader = std::make_shared<GLSLProgram>();
    shader->compileShaders(Paths::Shaders + "physicsDebug.vert",
        Paths::Shaders + "physicsDebug.frag");
    shader->linkShaders();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (const void*)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (const void*)offsetof(Vertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Runtime render — iterates live PhysX actors
// ---------------------------------------------------------------------------
void PhysicsDebugRenderer::Render(const RenderContext& ctx)
{
    if (!enabled) return;

    const auto& actors = physSystem.GetActors();
    if (actors.empty()) return;

    vertices.clear();

    Entity selected = EditorSelection::GetEntity();

    for (const auto& [entity, physActor] : actors)
    {
        if (!physActor || !physActor->actor) continue;

        glm::vec3 col = (entity == selected) ? COLOR_SELECTED : COLOR_DEFAULT;
        DrawActor(physActor->actor, col);
    }

    FlushAndDraw(ctx);
}

// ---------------------------------------------------------------------------
// Editor render — iterates ColliderComponents, no simulation needed
// ---------------------------------------------------------------------------
void PhysicsDebugRenderer::RenderFromComponents(const RenderContext& ctx, Scene& scene)
{
    if (!enabled) return;

    Registry& registry = scene.GetRegistry();

    auto& entities = registry.colliders.GetEntities();
    auto& dense = registry.colliders.GetDense();

    if (dense.empty()) return;

    vertices.clear();

    Entity selected = EditorSelection::GetEntity();

    for (size_t i = 0; i < dense.size(); ++i)
    {
        Entity entity = entities[i];
        const ColliderComponent& col = dense[i];

        if (col.shapes.empty()) continue;

        // Build a world matrix from the entity's TransformComponent.
        // If no transform exists, fall back to identity.
        glm::mat4 worldMatrix = glm::mat4(1.0f);
        if (registry.transforms.Has(entity))
        {
            const TransformComponent& t = registry.transforms.Get(entity);

            glm::vec3 pos = t.GetWorldPosition();
            glm::quat rot = t.localRotation;           // quat stored directly
            glm::vec3 scale = t.localScale;

            worldMatrix = glm::translate(glm::mat4(1.0f), pos)
                * glm::mat4_cast(rot)
                * glm::scale(glm::mat4(1.0f), scale);
        }

        glm::vec3 baseCol = (entity == selected) ? COLOR_SELECTED : COLOR_DEFAULT;
        DrawColliderComponent(col, worldMatrix, baseCol);
    }

    FlushAndDraw(ctx);
}

// ---------------------------------------------------------------------------
// Shared GPU upload + draw
// ---------------------------------------------------------------------------
void PhysicsDebugRenderer::FlushAndDraw(const RenderContext& ctx)
{
    if (vertices.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glm::mat4 viewProj = ctx.cameraProjection * ctx.cameraView;

    shader->use();
    shader->setMat4("u_ViewProj", viewProj);

    glBindVertexArray(vao);

    GLboolean depthWrite;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
    glDepthMask(GL_FALSE);

    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
    glLineWidth(1.0f);

    glDepthMask(depthWrite);

    glBindVertexArray(0);
    shader->unuse();
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void PhysicsDebugRenderer::EmitLine(const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& col)
{
    vertices.push_back({ a, col });
    vertices.push_back({ b, col });
}

void PhysicsDebugRenderer::EmitCircle(const glm::vec3& center,
    const glm::vec3& axisA,
    const glm::vec3& axisB,
    float radius,
    const glm::vec3& col,
    int segments)
{
    float step = static_cast<float>(2.0 * M_PI) / static_cast<float>(segments);
    glm::vec3 prev = center + axisA * radius;

    for (int i = 1; i <= segments; ++i)
    {
        float angle = static_cast<float>(i) * step;
        glm::vec3 curr = center
            + axisA * (radius * std::cos(angle))
            + axisB * (radius * std::sin(angle));
        EmitLine(prev, curr, col);
        prev = curr;
    }
}

// ===========================================================================
// Editor path — component-based, no PhysX required
// ===========================================================================

void PhysicsDebugRenderer::DrawColliderComponent(const ColliderComponent& collider,
    const glm::mat4& worldMatrix,
    const glm::vec3& baseCol)
{
    for (const ColliderShape& shape : collider.shapes)
    {
        glm::vec3 col = shape.isTrigger ? COLOR_TRIGGER : baseCol;
        DrawShapeFromData(shape, worldMatrix, col);
    }
}

void PhysicsDebugRenderer::DrawShapeFromData(const ColliderShape& shape,
    const glm::mat4& worldMatrix,
    const glm::vec3& col)
{
    switch (shape.type)
    {
    case ColliderShape::Type::Box:
    {
        // size stores full extents, DrawBox expects half-extents
        glm::vec3 halfExtents = shape.size * 0.5f;
        DrawBox(worldMatrix, halfExtents, col);
        break;
    }

    case ColliderShape::Type::Sphere:
        DrawSphere(worldMatrix, shape.radius, col);
        break;

    case ColliderShape::Type::Capsule:
        // PhysX convention: halfHeight is half the cylindrical shaft
        DrawCapsule(worldMatrix, shape.radius, shape.height * 0.5f, col);
        break;
    }
}

// ===========================================================================
// Shared geometry helpers (glm::mat4 overloads)
// ===========================================================================

void PhysicsDebugRenderer::DrawBox(const glm::mat4& world,
    const glm::vec3& h,
    const glm::vec3& col)
{
    glm::vec3 v[8] =
    {
        {-h.x, -h.y, -h.z}, { h.x, -h.y, -h.z},
        { h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z},
        {-h.x, -h.y,  h.z}, { h.x, -h.y,  h.z},
        { h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z},
    };

    for (auto& vert : v)
        vert = glm::vec3(world * glm::vec4(vert, 1.0f));

    // Bottom face
    EmitLine(v[0], v[1], col); EmitLine(v[1], v[2], col);
    EmitLine(v[2], v[3], col); EmitLine(v[3], v[0], col);
    // Top face
    EmitLine(v[4], v[5], col); EmitLine(v[5], v[6], col);
    EmitLine(v[6], v[7], col); EmitLine(v[7], v[4], col);
    // Vertical edges
    EmitLine(v[0], v[4], col); EmitLine(v[1], v[5], col);
    EmitLine(v[2], v[6], col); EmitLine(v[3], v[7], col);
}

void PhysicsDebugRenderer::DrawSphere(const glm::mat4& world,
    float radius,
    const glm::vec3& col)
{
    glm::vec3 center = glm::vec3(world[3]);
    glm::vec3 right = glm::normalize(glm::vec3(world[0]));
    glm::vec3 up = glm::normalize(glm::vec3(world[1]));
    glm::vec3 forward = glm::normalize(glm::vec3(world[2]));

    EmitCircle(center, right, up, radius, col);   // XY plane
    EmitCircle(center, right, forward, radius, col);   // XZ plane
    EmitCircle(center, forward, up, radius, col);   // YZ plane
}

void PhysicsDebugRenderer::DrawCapsule(const glm::mat4& world,
    float radius,
    float halfHeight,
    const glm::vec3& col)
{
    // Capsule primary axis is local X (PhysX convention)
    glm::vec3 center = glm::vec3(world[3]);
    glm::vec3 axisX = glm::normalize(glm::vec3(world[0]));
    glm::vec3 axisY = glm::normalize(glm::vec3(world[1]));
    glm::vec3 axisZ = glm::normalize(glm::vec3(world[2]));

    glm::vec3 top = center + axisX * halfHeight;
    glm::vec3 bottom = center - axisX * halfHeight;

    // Cylindrical shaft — 4 longitudinal lines + 2 cross-section rings
    EmitLine(top + axisY * radius, bottom + axisY * radius, col);
    EmitLine(top - axisY * radius, bottom - axisY * radius, col);
    EmitLine(top + axisZ * radius, bottom + axisZ * radius, col);
    EmitLine(top - axisZ * radius, bottom - axisZ * radius, col);

    EmitCircle(top, axisY, axisZ, radius, col);
    EmitCircle(bottom, axisY, axisZ, radius, col);

    // Hemispheres — two semi-circles per cap, in the two transverse planes
    auto EmitSemiCircle = [&](const glm::vec3& capCenter,
        const glm::vec3& primary,
        const glm::vec3& a,
        const glm::vec3& b)
        {
            int   seg = CIRCLE_SEGMENTS / 2;
            float step = static_cast<float>(M_PI) / static_cast<float>(seg);
            glm::vec3 prev = capCenter + primary * radius;

            for (int i = 1; i <= seg; ++i)
            {
                float angle = static_cast<float>(i) * step;
                glm::vec3 curr = capCenter
                    + primary * (radius * std::cos(angle))
                    + a * (radius * std::sin(angle));
                EmitLine(prev, curr, col);
                prev = curr;
            }

            prev = capCenter + primary * radius;
            for (int i = 1; i <= seg; ++i)
            {
                float angle = static_cast<float>(i) * step;
                glm::vec3 curr = capCenter
                    + primary * (radius * std::cos(angle))
                    + b * (radius * std::sin(angle));
                EmitLine(prev, curr, col);
                prev = curr;
            }
        };

    EmitSemiCircle(top, axisX, axisY, axisZ);
    EmitSemiCircle(bottom, -axisX, axisY, axisZ);
}

// ===========================================================================
// Runtime path — PxGeometry overloads (delegate to glm::mat4 overloads)
// ===========================================================================

void PhysicsDebugRenderer::DrawActor(PxRigidActor* actor, const glm::vec3& col)
{
    PxU32 count = actor->getNbShapes();
    if (count == 0) return;

    constexpr PxU32 STACK_MAX = 64;
    PxShape* stackBuf[STACK_MAX];
    std::vector<PxShape*> heapBuf;
    PxShape** shapes = stackBuf;

    if (count > STACK_MAX)
    {
        heapBuf.resize(count);
        shapes = heapBuf.data();
    }

    actor->getShapes(shapes, count);
    for (PxU32 i = 0; i < count; ++i)
        DrawShape(actor, shapes[i], col);
}

void PhysicsDebugRenderer::DrawShape(PxRigidActor* actor,
    PxShape* shape,
    const glm::vec3& col)
{
    glm::vec3 effectiveCol = shape->getFlags().isSet(PxShapeFlag::eTRIGGER_SHAPE)
        ? COLOR_TRIGGER : col;

    PxTransform      world = actor->getGlobalPose() * shape->getLocalPose();
    PxGeometryHolder geo = shape->getGeometry();

    switch (geo.getType())
    {
    case PxGeometryType::eBOX:
        DrawBox(world, geo.box(), effectiveCol);
        break;
    case PxGeometryType::eSPHERE:
        DrawSphere(world, geo.sphere(), effectiveCol);
        break;
    case PxGeometryType::eCAPSULE:
        DrawCapsule(world, geo.capsule(), effectiveCol);
        break;
    default:
        break;
    }
}

// PxTransform overloads — convert to glm::mat4 and forward
void PhysicsDebugRenderer::DrawBox(const PxTransform& t,
    const PxBoxGeometry& geo,
    const glm::vec3& col)
{
    glm::vec3 h = { geo.halfExtents.x, geo.halfExtents.y, geo.halfExtents.z };
    DrawBox(PxToGLM(t), h, col);
}

void PhysicsDebugRenderer::DrawSphere(const PxTransform& t,
    const PxSphereGeometry& geo,
    const glm::vec3& col)
{
    DrawSphere(PxToGLM(t), geo.radius, col);
}

void PhysicsDebugRenderer::DrawCapsule(const PxTransform& t,
    const PxCapsuleGeometry& geo,
    const glm::vec3& col)
{
    DrawCapsule(PxToGLM(t), geo.radius, geo.halfHeight, col);
}