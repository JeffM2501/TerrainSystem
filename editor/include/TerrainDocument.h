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
	std::unordered_map<std::string, TerrainMaterial> MaterialLibrary;
	float SunVector[3] = { 0,0,1 };

	TerrainTile& GetTile(int x, int y);
	bool HasTile(int x, int y) const;

	void LoadMaterial(const std::string& name, std::string_view path);

	const TerrainMaterial* GetMaterial(const std::string& name) const;

	TerrainPosition SelectedTileLoc;

	TerrainPosition TerrainBounds = { 0,0 };

protected:
	void OnAssetOpen() override;
	void OnAssetDirty() override;

	void SetupDocument();

	bool ShowSplat = false;

	Shader TerrainShader = { 0 };
	TerainRenderer Renderer;
	int SunVectorLoc = 0;
	int SelectedShaderFlagLoc = 0;
	int ShowSplatFlagLoc = 0;
};