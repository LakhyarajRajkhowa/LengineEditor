#pragma once

#include "graphics/renderer/RenderPipeline.h"
#include "physics/PhysicsSystem.h"
#include "core/EditorSelection.h"

using namespace Lengine;

class DebugDraw
{
public:
    
    static void Line(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);

    static void WireCube(const glm::mat4& transform,
        const glm::vec3& size,
        const glm::vec3& color = { 1, 1, 0 });

    struct DebugLine
    {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 color;
    };

    void Init();
    void Render(RenderContext& ctx)
    {
        if (lines.empty()) return;


        shader->use();
        shader->setMat4("u_ViewProj", ctx.cameraView);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));

        shader->unuse();

        lines.clear();
    }

    static std::vector<DebugLine> lines;

private:
    GLuint debugVAO = 0;
    GLuint debugVBO = 0;
    std::shared_ptr<GLSLProgram> shader; // Assume you assign a shader somewhere
};



class PhysicsDebugRenderer
{
public:
    PhysicsDebugRenderer(PhysicsSystem& physSystem)
        : physSystem(physSystem) {
        debugDraw.Init();
    }

    void Render(RenderContext& ctx)
    {
        Lengine::UUID selected = EditorSelection::GetEntity();

        if (!physSystem.GetActors().count(selected))
            return;

        const auto& actor = physSystem.GetActors().at(selected);

        DrawActor(actor->actor);

        debugDraw.Render(ctx);
    }

private:
    PhysicsSystem& physSystem;
    DebugDraw debugDraw;

    void DrawActor(PxRigidActor* actor)
    {
        PxU32 count = actor->getNbShapes();
        PxShape* shapes[32]; // Max 32 shapes per actor
        actor->getShapes(shapes, count);

        for (PxU32 i = 0; i < count; i++)
        {
            DrawShape(actor, shapes[i]);
        }
    }

    void DrawShape(PxRigidActor* actor, PxShape* shape)
    {
        PxTransform actorPose = actor->getGlobalPose();
        PxTransform localPose = shape->getLocalPose();

        PxTransform world = actorPose * localPose;

        PxGeometryHolder geo = shape->getGeometry();

        switch (geo.getType())
        {
        case PxGeometryType::eBOX:
            DrawBox(world, geo.box());
            break;

        case PxGeometryType::eSPHERE:
            // DrawSphere(world, geo.sphere());
            break;

        case PxGeometryType::eCAPSULE:
            // DrawCapsule(world, geo.capsule());
            break;
        }
    }

    void DrawBox(const PxTransform& t, const PxBoxGeometry& geo)
    {
        glm::mat4 world = PxToGLM(t);

        glm::vec3 size = {
            geo.halfExtents.x * 2,
            geo.halfExtents.y * 2,
            geo.halfExtents.z * 2
        };

        DebugDraw::WireCube(world, size);
    }
};