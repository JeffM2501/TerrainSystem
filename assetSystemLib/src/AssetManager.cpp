#include "AssetManager.h"

#include "AssetDatabase.h"
#include "AssetFile.h"

namespace AssetSystem
{
    struct AssetFileInfo
    {
        std::string FilePath;
        AssetFile::Ptr AssetData;
        bool IsDirty = false;
        int OpenCount = 0;
    };

    static std::unordered_map<uint64_t, std::function<std::shared_ptr<AssetFile>()>> AssetFactories;
    static std::unordered_map<GUID, AssetFileInfo> LoadedAssets;


    AssetFile::Ptr AssetManager::OpenAsset(const GUID& assetId)
    {
        auto assetInfo = LoadedAssets.find(assetId);
        if (assetInfo != LoadedAssets.end())
        {
            assetInfo->second.OpenCount++;
            return assetInfo->second.AssetData;
        }

        return nullptr;
    }
    AssetFile::Ptr AssetManager::OpenAsset(const std::string& assetPath)
    {
        GUID* assetId = AssetDB::GetGuidForFile(assetPath);

        if (assetId)
        {
            auto assetInfo = LoadedAssets.find(*assetId);
            if (assetInfo != LoadedAssets.end())
            {
                assetInfo->second.OpenCount++;
                return assetInfo->second.AssetData;
            }
        }
        // Open asset and register it for tracking

        // find the asset type

        return nullptr;
    }

    AssetFile::Ptr AssetManager::CloseAsset(const GUID& assetId)
    {
        auto assetInfo = LoadedAssets.find(assetId);
        if (assetInfo != LoadedAssets.end())
        {
            assetInfo->second.OpenCount--;
            if (assetInfo->second.OpenCount == 0)
            {
                assetInfo->second.AssetData->Unload();
                m_loadedAssets.erase(assetInfo);
            }
        }
        return nullptr;
    }
    void AssetManager::RegisterAssetType(uint64_t assetTypeId, std::function<AssetFile::Ptr()> factory)
    {
        m_assetFactories[assetTypeId] = factory;
    }
    AssetFile::Ptr AssetManager::CreateAsset(uint64_t assetTypeId)
    {
        auto factory = m_assetFactories.find(assetTypeId);
        if (factory != m_assetFactories.end())
        {
            return factory->second();
        }
        return nullptr;
    }
}