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
            std::string Path;
            uint64_t PathHash;
            uint64_t AssetTypeId;
        };

        size_t GetAssetsOfType(uint64_t assetTypeId, std::vector<const AssetFileRecord*>& outAssets);
    }
}