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
            s_SelectedAsset = asset;
        }

        static const std::pair<UUID, AssetType> GetAsset()
        {
            return s_SelectedAsset;
        }

        static bool HasAsset()
        {
            return s_SelectedAsset.first != UUID::Null;
        }

        static void ClearAssetSelection()
        {
            
            s_SelectedAsset.first = UUID::Null;
        }

        static void SetEntity(Entity id)
        {
            s_SelectedEntity = id;
        }

        static Entity GetEntity()
        {
            return s_SelectedEntity;
        }

        static bool HasEntity()
        {
            return s_SelectedEntity != NullEntity;
        }

        static void ClearEntitySelection()
        {
            s_SelectedEntity = NullEntity;
        }

    private:
        static inline std::pair<UUID , AssetType> s_SelectedAsset = std::pair(UUID::Null, AssetType::Unknown);
        static bool isAssetSaved;
        static inline Entity s_SelectedEntity = NullEntity;
    };


}
