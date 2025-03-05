#pragma once


#include "GUID.h"
#include "AssetFile.h"
#include <memory>
#include <unordered_map>
#include <functional>

namespace AssetSystem
{
    class AssetManager
    {
    public:
        template<typename T>
        static std::shared_ptr<T> OpenAsset(const GUID& assetId)
        {
            return std::dynamic_pointer_cast<T>(OpenAsset(assetId));
        }
        static AssetFile::Ptr OpenAsset(const GUID& assetId);

        template<typename T>
        static std::shared_ptr<T> OpenAsset(const std::string& assetPath)
        {
            return std::dynamic_pointer_cast<T>(OpenAsset(assetPath));
        }

        static AssetFile::Ptr OpenAsset(const std::string& assetPath);

        static AssetFile::Ptr CloseAsset(const GUID& assetId);

        static void RegisterAssetType(uint64_t assetTypeId, std::function<AssetFile::Ptr()> factory);

        template<class T>
        static void RegisterAssetType()
        {
            RegisterAssetType(T::AssetTypeID(), T::Factory);
        }

        static AssetFile::Ptr CreateAsset(uint64_t assetTypeId);

        template<class T>
        static AssetFile::Ptr CreateAsset()
        {
            return CreateAsset(T::AssetTypeID());
        }
    };
}