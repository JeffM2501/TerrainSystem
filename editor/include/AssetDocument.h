#pragma once

#include "Document.h"

#include "types/asset.h"
#include "AssetDatabase.h"

#include "raylib.h"
#include <vector>
#include <string>

template<class T>
class AssetDocument : public EditorFramework::ViewportDocument
{
public:
    T* AssetData = nullptr;

    inline uint64_t GetDocumentTypeID() override { return 0; }
    inline std::string_view GetFileExtension()override { return ".asset"; }

    std::string_view GetDocumentName() override
    {
        if (!AssetData)
            return "Unknown";

        if (AssetPath.empty())
        {
            auto* info = AssetSystem::AssetFileDatabase::GetAsset(AssetData->GetMeta().GetGUID());
            AssetPath = info->Path;
        }
        
        return GetFileName(AssetPath.c_str());
    }

protected:
    std::string AssetPath;
};