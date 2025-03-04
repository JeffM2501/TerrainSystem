#include "AssetDatabase.h"

#include <unordered_map>
#include <string>

namespace AssetSystem
{
    static std::unordered_map<GUID, std::string> AssetsById;
    static std::unordered_map<std::string, GUID> AssetsByPath;

    GUID* AssetDB::GetGuidForFile(const std::string& filePath) 
    {
        auto asset = AssetsByPath.find(filePath);
        if (asset != AssetsByPath.end())
        {
            return &asset->second;
        }
        return nullptr;
    }

    const std::string& AssetDB::GetFilePathForGuid(const GUID& guid)
    {
        auto asset = AssetsById.find(guid);
        if (asset != AssetsById.end())
        {
            return asset->second;
        }
        return "";
    }

    void AssetDB::AddAssetFile(const GUID& guid, const std::string& filePath)
    {
        AssetsById[guid] = filePath;
        AssetsByPath[filePath] = guid;
    }
}