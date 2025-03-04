#pragma once

#include "GUID.h"

namespace AssetSystem
{
    class AssetInfo
    {
    public:
        GUID AssetId = GUID::Generate();

        uint64_t AssetTypeID = 0;
        uint64_t ContentHash = 0;
    };

    class AssetFile
    {
    public:
        AssetInfo Info;
    };
}