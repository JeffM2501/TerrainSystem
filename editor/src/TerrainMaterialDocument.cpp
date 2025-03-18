#include "TerrainMaterialDocument.h"

#include "AssetManager.h"

using namespace AssetSystem;

std::string_view TerrainMaterialDocument::GetDocumentName()
{
    if (AssetData && !AssetData->GetPath().empty())
        return GetFileNameWithoutExt(AssetData->GetPath().c_str());

    static std::string unknownName;
    unknownName = "Untitled ";
    unknownName += AssetTypes::TerrainMaterialAsset::TypeName;

    return unknownName;
}

void TerrainMaterialDocument::CreateAsset()
{
    AssetData = AssetManager::CreateTempAsset<AssetTypes::TerrainMaterialAsset>();
}

void TerrainMaterialDocument::OpenAsset(const std::string& assetPath)
{
    AssetData = AssetManager::OpenAsset<AssetTypes::TerrainMaterialAsset>(assetPath);
}

void TerrainMaterialDocument::SaveAsset()
{

}

void TerrainMaterialDocument::SaveAsAsset(const std::string& assetPath)
{

}

void TerrainMaterialDocument::OnShowScene(const Vector2& renderSize)
{

}