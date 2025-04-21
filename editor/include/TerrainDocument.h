#pragma once

#include "Document.h"

#include "TerrainTile.h"
#include "TerrainRender.h"
#include "AssetDocument.h"

#include "types/terrain.h"

#include "raylib.h"
#include <vector>
#include <string>

class TerrainDocument : public AssetDocument<AssetTypes::TerrainAsset>
{
public:
	REGISTER_DOCUMENT(TerrainDocument, terrain);
	void OnUpdate(int width, int height);
	void OnShowScene(const Vector2& renderSize) override;
	void OnShowUI() override;
	void OnCreated() override;

	TerrainInfo Info;
	std::vector<TerrainTile> Tiles;

	std::vector<TerrainMaterial> MaterialListCache;

	float SunVector[3] = { 0,0,1 };

	TerrainTile& GetTile(int x, int y);
	bool HasTile(int x, int y) const;

	TerrainPosition SelectedTileLoc;

	TerrainPosition TerrainBounds = { 0,0 };

protected:
	void OnAssetCreate() override;
	void OnAssetOpen() override;
	void OnAssetDirty() override;

	void SetupDocument();

	void HandleMaterialListChangedEvent(const ValueChangedEvent& event);

	void RebuildMaterialIndex(int index);

	bool ShowSplat = false;

	Shader TerrainShader = { 0 };
	TerainRenderer Renderer;
	int SunVectorLoc = 0;
	int SelectedShaderFlagLoc = 0;
	int ShowSplatFlagLoc = 0;

	std::vector<std::unique_ptr<AssetReferenceResolver<AssetTypes::TerrainMaterialAsset>>> MaterialRefs;
};