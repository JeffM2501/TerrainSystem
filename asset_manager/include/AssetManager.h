#pragma once

#include <memory>
#include <functional>
#include <cstdint>

#include "GUID.h"
#include "CRC64.h"

namespace AssetSystem
{
    struct AssetMetaInfo
    {
        Hashes::GUID Guid = Hashes::GUID::Invalid();
        size_t AssetTypeId = 0;
    };

    class Asset
    {
    public:
        AssetMetaInfo Meta;
        virtual uint64_t GetAssetTypeId() = 0;

        using Ptr = std::shared_ptr<Asset>;
    };

#define REGISTER_ASSET(T) \
	static Asset::Ptr Factory() { return std::make_unique<T>(); } \
	static uint64_t AssetTypeID() { return Hashes::CRC64Str(#T); } \
	static std::string_view GetAssetTypeName() { return #T; } \
	inline uint64_t GetAssetTypeId() override { return T::AssetTypeID(); } 

    namespace AsseTypetManager
    {
        void RegisterAssetType(std::function<Asset::Ptr()> factory, uint64_t assetTypeId, std::string_view typeNames);

        template<class T>
        void ReisterAssetType()
        {
            RegisterAssetType(T::Factory, T::AssetTypeID(), T::GetAssetTypeName());
        }

        Asset::Ptr CreateAsset(uint64_t assetTypeId);
    }

    namespace AssetManager
    {
        Asset::Ptr OpenAsset(std::string_view assetFilePath);
        Asset::Ptr OpenAssetRef(const Hashes::GUID& assetGUID);

        void CloseAsset(Asset::Ptr asset);

        template <class T>
        std::shared_ptr<T> OpenAsset(std::string_view assetFilePath)
        {
            auto asset = OpenAsset(assetFilePath);
            if(asset)
            {
                if (asset->GetAssetTypeId != T::AssetTypeID())
                {
                    CloseAsset(asset);
                    return nullptr;
                }

                return static_pointer_cast<T>(asset);
            }

            return nullptr;
        }

        template <class T>
        std::shared_ptr<T> OpenAssetRef(const Hashes::GUID& assetGUID)
        {
            auto asset = OpenAssetRef(assetGUID);
            if (asset)
            {
                if (asset->GetAssetTypeId != T::AssetTypeID())
                {
                    CloseAsset(asset);
                    return nullptr;
                }

                return static_pointer_cast<T>(asset);
            }

            return nullptr;
        }
    }
}