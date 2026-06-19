#pragma once

// Engine
#include "scene/Entity.h"
#include "assets/AssetRegistry.h"

namespace Lengine {
    class EditorSelection
    {
    public:
        static void SetAsset(const std::pair<UUID, AssetType>& asset)
        {
            selectedAsset = asset;
        }

        static const std::pair<UUID, AssetType> GetAsset()
        {
            return selectedAsset;
        }

        static bool HasAsset()
        {
            return selectedAsset.first != UUID::Null;
        }

        static void ClearAssetSelection()
        {
            
            selectedAsset.first = UUID::Null;
        }

        static void SetEntity(Entity id)
        {
            selectedEntity = id;
        }

        static Entity GetEntity()
        {
            return selectedEntity;
        }

        static bool HasEntity()
        {
            return selectedEntity != NullEntity;
        }

        static void ClearEntitySelection()
        {
            selectedEntity = NullEntity;
        }

        static void SetCopiedEntity(const Entity e) {
            copiedEntity = e;
        }

        static Entity GetCopiedEntity() {
            return copiedEntity;
        }

    private:
        static inline std::pair<UUID , AssetType> selectedAsset = std::pair(UUID::Null, AssetType::Unknown);
        static bool isAssetSaved;
        static inline Entity selectedEntity = NullEntity;
        static inline Entity copiedEntity = NullEntity;
    };


}
