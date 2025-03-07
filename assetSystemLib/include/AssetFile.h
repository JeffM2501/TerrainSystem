#pragma once

#include "GUID.h"
#include "CRC64.h"

#include <memory>

namespace AssetSystem
{
    class AssetInfo
    {
    public:
        GUID AssetId = GUID::Generate();

        uint64_t AssetTypeID = 0;
    };

    class AssetFile
    {
    public:
        AssetInfo Info;

        virtual uint64_t GetAssetTypeID() const { return Info.AssetTypeID; }

        using Ptr = std::shared_ptr<AssetFile>;
    };
}

#define REGISTER_ASSET(T) \
	static std::shared_ptr<T> Factory() { return std::make_shared<T>(); } \
	static uint64_t AssetTypeID() { return Hashes::CRC64Str(#T); } \
    T() { Info.AssetTypeID = AssetTypeID(); }
