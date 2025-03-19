#pragma once

#include "Document.h"

#include "TerrainTile.h"

#include "types/terrain.h"
#include "CRC64.h"

#include "raylib.h"
#include <vector>
#include <string>

class TerrainMaterialDocument : public EditorFramework::ViewportDocument
{
public:
    REGISTER_DOCUMENT(TerrainMaterialDocument, terrainMaterial);
    std::string_view GetDocumentName();

    void CreateAsset() override;
    void OpenAsset(const std::string& assetPath) override;
    void SaveAsset() override;
    void SaveAsAsset(const std::string& assetPath) override;

	void OnCreated() override;
	void OnActivated() override;
	void OnDeactivated() override;

protected:
   void OnShowScene(const Vector2& renderSize) override;

protected:
    AssetTypes::TerrainMaterialAsset* AssetData = nullptr;
};