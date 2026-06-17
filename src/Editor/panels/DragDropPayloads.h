#pragma once

// Engine
#include "utils/UUID.h"


namespace Lengine {
    struct MeshDragPayload
    {
        UUID id;
        char path[512];
    };
    struct TextureDragPayload
    {
        UUID id;
        char path[512];
    };
    struct MaterialDragPayload
    {
        UUID id;
        char path[512];
    };
    struct BoneMaskDragPayload
    {
        UUID id;
    };
    struct SkeletonDragPayload
    {
        UUID id;
        char path[512];
    };

    struct ScriptDragPayload
    {
        char name[64];
        char sourceFile[256];
    };
}
