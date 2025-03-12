#include "AssetDatabase.h"
#include "GUID.h"
#include <unordered_map>

namespace AssetSystem
{
    namespace AssetFileDatabase
    {
        static std::unordered_map<Hashes::GUID, AssetFileRecord> AssetRecords;
        static std::unordered_map<size_t, Hashes::GUID> AssetsByPath;

        const AssetFileRecord* GetAsset(const Hashes::GUID& guid)
        {
            auto it = AssetRecords.find(guid);
            if (it != AssetRecords.end())
                return &it->second;
            return nullptr;
        }

        const AssetFileRecord* GetAsset(std::string_view path)
        {
            size_t pathHash = std::hash<std::string_view>{}(path);
            return GetAsset(pathHash);
        }

        const AssetFileRecord* GetAsset(size_t pathHash)
        {
            auto it = AssetsByPath.find(pathHash);
            if (it != AssetsByPath.end())
                return GetAsset(it->second);
            return nullptr;
        }

        void AddAsset(const Hashes::GUID& guid, std::string_view path, size_t assetTypeId)
        {
            AssetFileRecord record;
            record.Guid = guid;
            record.Path = path;
            record.PathHash = std::hash<std::string_view>{}(path);
            record.AssetTypeId = assetTypeId;
            AssetRecords[guid] = record;
            AssetsByPath[record.PathHash] = guid;
        }

        size_t GetAssetsOfType(uint64_t assetTypeId, std::vector<const AssetFileRecord*>& outAssets)
        {
            outAssets.clear();
            for (auto& [guid, record] : AssetRecords)
            {
                if (record.AssetTypeId == assetTypeId)
                    outAssets.push_back(&record);
            }
            return outAssets.size();
        }
    }
}