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
        extern TypeDatabase TypeDB;

        AssetTypes::Asset* FindExistingAsset(const std::string& assetFilePath);
        AssetTypes::Asset* FindExistingAsset(const Hashes::GUID& assetGUID);

        void StoreAsset(const std::string& assetFilePath, std::shared_ptr<TypeWraper> assetData);

        void CloseAsset(AssetTypes::Asset* asset);

        template <class T>
        T* OpenAsset(const std::string& assetFilePath)
        {
            auto existing = FindExistingAsset(assetFilePath);

            if (existing)
            {
                if (existing->TypeName != T::TypeName)
                    return nullptr;
                return static_cast<T*>(existing);
            }
            
            std::shared_ptr<T> asset = std::make_shared<T>();
            asset->ReadAs<T>(assetFilePath, TypeDB, true);

            auto asset = OpenAsset(assetFilePath);
            if(!asset->IsValid())
            {
                return nullptr;
            }

            StoreAsset(assetFilePath, asset);

            return asset.get();
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