#pragma once

#include "GUID.h"
#include <string>
#include <vector>

namespace AssetSystem
{
    namespace AssetFileDatabase
    {
        struct AssetFileRecord
        {
            GUID Guid;
            std::string Path;
            size_t PathHash;
            size_t AssetTypeId;
        };

        const AssetFileRecord* GetAsset(const GUID& guid);
        const AssetFileRecord* GetAsset(std::string_view path);
        const AssetFileRecord* GetAsset(size_t pathHash);

        void AddAsset(const GUID& guid, std::string_view path, size_t assetTypeId);

        size_t GetAssetsOfType(size_t assetTypeId, std::vector<const AssetFileRecord*>& outAssets);
    }
}