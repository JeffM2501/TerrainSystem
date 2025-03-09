#pragma once

#include "GUID.h"

namespace AssetSystem
{
    struct AssetMetaInfo
    {
        GUID Guid = GUID::Invalid();
        size_t AssetTypeId = 0;
    };

    class Asset
    {
    public:
        AssetMetaInfo Meta;
        virtual size_t GetAssetTypeId() = 0;
    };

#define REGISTER_ASSET(T) \
	static EditorFramework::DocumentPtr Factory() { return std::make_unique<T>(); } \
	static size_t DocumentTypeID() { return Hashes::CRC64Str(#T); } \
	static std::string_view FileExtension() { return #E; } \
	static std::string_view GetDocumentTypeName() { return #T; } \
	inline size_t GetDocumentTypeID() override { return T::DocumentTypeID(); } \
	inline std::string_view GetFileExtension()override { return #E; }


    namespace AssetManager
    {

    }
}