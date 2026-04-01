#pragma once

#include "utils/UUID.h"
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

        static void SetEntity(UUID id)
        {
            s_SelectedEntity = id;
        }

        static UUID GetEntity()
        {
            return s_SelectedEntity;
        }

        static bool HasEntity()
        {
            return s_SelectedEntity != UUID::Null;
        }

        static void ClearEntitySelection()
        {
            s_SelectedEntity = UUID::Null;
        }

    private:
        static inline std::pair<UUID , AssetType> s_SelectedAsset = std::pair(UUID::Null, AssetType::Unknown);
        static bool isAssetSaved;
        static inline UUID s_SelectedEntity = UUID::Null;
    };


}
