#pragma once

#include <memory>
#include <functional>
#include <cstdint>

#include "GUID.h"
#include "CRC64.h"

#include "types/asset.h"

namespace AssetSystem
{
    namespace AssetManager
    {
        AssetTypes::Asset* FindExistingAsset(std::string_view assetFilePath);
        AssetTypes::Asset* FindExistingAsset(const Hashes::GUID& assetGUID);

        void CloseAsset(AssetTypes::Asset* asset);

        template <class T>
        T* OpenAsset(std::string_view assetFilePath)
        {
            auto asset = OpenAsset(assetFilePath);
            if(asset)
            {
                if (asset->TypeName != T::TypeName)
                {
                    CloseAsset(asset);
                    return nullptr;
                }

                return static_pointer_cast<T>(asset);
            }

            return nullptr;
        }

        template <class T>
        T* OpenAssetRef(const Hashes::GUID& assetGUID)
        {
            auto asset = OpenAssetRef(assetGUID);
            if (asset)
            {
                if (asset->TypeName != T::TypeName)
                {
                    CloseAsset(asset);
                    return nullptr;
                }

                return static_pointer_cast<T>(asset);
            }

            return nullptr;
        }
    }
}