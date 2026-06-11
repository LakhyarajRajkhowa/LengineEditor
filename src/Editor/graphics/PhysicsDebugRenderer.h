#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// Editor
#include "Editor/core/EditorSelection.h"               

// Engine
#include "graphics/renderer/RenderPipeline.h"  
#include "physics/PhysicsSystem.h"              
#include "scene/components/ColliderComponent.h"          
#include "scene/scene.h"                        

namespace Lengine {

    // -----------------------------------------------------------------------
    //  PhysicsDebugRenderer
    //
    //  Two rendering modes:
    //
    //  1) Runtime  – call Render(ctx)
    //     Iterates PhysicsSystem::GetActors() and draws whatever PxShapes are
    //     live in the simulation.  Requires physics to be running.
    //
    //  2) Editor   – call RenderFromComponents(ctx, scene)
    //     Iterates every ColliderComponent in the scene registry and builds
    //     wireframes purely from component data (type / size / radius / height).
    //     No PhysX simulation needed; works while the scene is at rest in the
    //     editor.
    //
    //  Selected entities are drawn in green; triggers in cyan; rest in yellow.
    // -----------------------------------------------------------------------
    class PhysicsDebugRenderer
    {
    public:
        // Color scheme
        static constexpr glm::vec3 COLOR_DEFAULT = { 1.0f, 1.0f, 0.0f };   // yellow
        static constexpr glm::vec3 COLOR_SELECTED = { 0.1f, 1.0f, 0.2f };   // green
        static constexpr glm::vec3 COLOR_TRIGGER = { 0.0f, 1.0f, 1.0f };   // cyan

        // Capsule / sphere tessellation quality
        static constexpr int CIRCLE_SEGMENTS = 32;

        explicit PhysicsDebugRenderer(PhysicsSystem& physSystem);
        ~PhysicsDebugRenderer();

        // Call once after an OpenGL context is available
        void Init();

        // ---- Runtime render (physics simulation must be active) ----
        // Iterates live PxActors registered in PhysicsSystem.
        void Render(const RenderContext& ctx);

        // ---- Editor render (no simulation required) ----
        // Iterates ColliderComponents in the scene registry and reconstructs
        // world-space wireframes from component data + TransformComponent.
        void RenderFromComponents(const RenderContext& ctx, Scene& scene);

        bool enabled = true;

    private:
        // ---- GPU resources ----
        GLuint vao = 0;
        GLuint vbo = 0;
        std::shared_ptr<GLSLProgram> shader;

        // ---- CPU line buffer (rebuilt every frame) ----
        struct Vertex { glm::vec3 pos; glm::vec3 color; };
        std::vector<Vertex> vertices;   // pairs of vertices == lines

        PhysicsSystem& physSystem;

        // ---- GPU flush & draw (shared by both render paths) ----
        void FlushAndDraw(const RenderContext& ctx);

        // ---- line emit helpers ----
        void EmitLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& col);
        void EmitCircle(const glm::vec3& center,
            const glm::vec3& axisA,
            const glm::vec3& axisB,
            float radius,
            const glm::vec3& col,
            int segments = CIRCLE_SEGMENTS);

        // ---- Runtime draw path (PxActor-based) ----
        void DrawActor(PxRigidActor* actor, const glm::vec3& col);
        void DrawShape(PxRigidActor* actor, PxShape* shape, const glm::vec3& col);

        // ---- Editor draw path (component-based, no PhysX) ----
        void DrawColliderComponent(const ColliderComponent& collider,
            const glm::mat4& worldMatrix,
            const glm::vec3& col);
        void DrawShapeFromData(const ColliderShape& shape,
            const glm::mat4& worldMatrix,
            const glm::vec3& col);

        // ---- Per-geometry helpers (shared by both paths) ----
        void DrawBox(const glm::mat4& world, const glm::vec3& halfExtents, const glm::vec3& col);
        void DrawSphere(const glm::mat4& world, float radius, const glm::vec3& col);
        void DrawCapsule(const glm::mat4& world, float radius, float halfHeight, const glm::vec3& col);

        // ---- Legacy PxGeometry overloads (used by runtime path) ----
        void DrawBox(const PxTransform& world, const PxBoxGeometry& geo, const glm::vec3& col);
        void DrawSphere(const PxTransform& world, const PxSphereGeometry& geo, const glm::vec3& col);
        void DrawCapsule(const PxTransform& world, const PxCapsuleGeometry& geo, const glm::vec3& col);
    };

} // namespace Lengine