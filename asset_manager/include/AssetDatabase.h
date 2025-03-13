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
            Hashes::GUID Guid;
            std::string Path;
            uint64_t PathHash;
            uint64_t AssetTypeId;
        };

        const AssetFileRecord* GetAsset(const Hashes::GUID& guid);
        const AssetFileRecord* GetAsset(std::string_view path);

        void AddAsset(const Hashes::GUID& guid, std::string_view path, uint64_t assetTypeId);

        size_t GetAssetsOfType(uint64_t assetTypeId, std::vector<const AssetFileRecord*>& outAssets);
    }
}