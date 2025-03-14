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

        void CloseAsset(AssetTypes::Asset* asset);

        void StoreAsset(const std::string& assetFilePath, std::shared_ptr<TypeWraper> assetData);

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
            // todo make the path relative to asset root not working dir
            asset->ReadAs<T>(assetFilePath, TypeDB, true);
            if(!asset->IsValid())
            {
                return nullptr;
            }

            // set the current path in the asset
            // TODO, see if the path is different?
            asset->SetPath(assetFilePath);

            StoreAsset(assetFilePath, asset);

            return asset.get();
        }
    }
}