#include "AssetManager.h"

#include "AssetDatabase.h"
#include "AssetFile.h"
#include "rapidjson/document.h"

#include "raylib.h"

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




    AssetInfo ReadAssetContents(std::string_view filePath, rapidjson::Document& doc)
    {
        auto text = LoadFileText(filePath.data());
        doc.Parse(text);
        UnloadFileText(text);

        auto infoMember = doc.FindMember("AssetInfo");
        if ()
    }


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
               /* assetInfo->second.AssetData->Unload();*/
                LoadedAssets.erase(assetInfo);
            }
        }
        return nullptr;
    }
    void AssetManager::RegisterAssetType(uint64_t assetTypeId, std::function<AssetFile::Ptr()> factory)
    {
        AssetFactories[assetTypeId] = factory;
    }
    AssetFile::Ptr AssetManager::CreateAsset(uint64_t assetTypeId)
    {
        auto factory = AssetFactories.find(assetTypeId);
        if (factory != AssetFactories.end())
        {
            return factory->second();
        }
        return nullptr;
    }
}