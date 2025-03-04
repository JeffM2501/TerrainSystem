#pragma once

#include "GUID.h"
namespace AssetSystem
{
    class AssetDB
    {
    public:
        static GUID* GetGuidForFile(const std::string& filePath);
        static const std::string& GetFilePathForGuid(const GUID& guid);
        static void AddAssetFile(const GUID& guid, const std::string& filePath);
    };
}