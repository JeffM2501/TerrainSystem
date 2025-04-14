#include "TerrainMaterialDocument.h"

#include <PropertiesPanel.h>
#include "AssetManager.h"
#include "Application.h"

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
    AssetData = EditManager.CreateTempAsset<AssetTypes::TerrainMaterialAsset>();
    RegisterEditHandler();
}

void TerrainMaterialDocument::OpenAsset(const std::string& assetPath)
{
    AssetData = EditManager.OpenAsset<AssetTypes::TerrainMaterialAsset >(assetPath);
    RegisterEditHandler();
}

void TerrainMaterialDocument::RegisterEditHandler()
{
    AssetData->ValuePtr->OnValueChanged.Add([this](const Types::ValueChangedEvent& event)
        {
            SetDirty();
        }, Token.GetToken());
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

void TerrainMaterialDocument::OnCreated()
{
    ClearColor = BLACK;
}

void TerrainMaterialDocument::OnActivated()
{
    auto* properties = GetApp()->GetPanel<PropertiesPanel>();
    if (properties && AssetData)
        properties->SetObject(AssetData->ValuePtr);

    EditManager.Activate();
}

void TerrainMaterialDocument::OnDeactivated()
{
    auto* properties = GetApp()->GetPanel<PropertiesPanel>();
    if (properties)
        properties->SetObject(nullptr);

    EditManager.Deactivate();
}