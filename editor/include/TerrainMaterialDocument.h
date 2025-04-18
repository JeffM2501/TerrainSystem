#pragma once

#include "Document.h"
#include "AssetDocument.h"

#include "TerrainTile.h"
#include "LifetimeToken.h"
#include "AssetEditManager.h"

#include "types/terrain.h"
#include "CRC64.h"

#include "raylib.h"
#include <vector>
#include <string>

class TerrainMaterialDocument : public AssetDocument<AssetTypes::TerrainMaterialAsset>
{
public:
	REGISTER_DOCUMENT(TerrainMaterialDocument, terrainMaterial);

	void OnCreated() override;

	~TerrainMaterialDocument();
protected:
	void OnShowScene(const Vector2& renderSize) override;

	void RebuildMaterial();

	void OnAssetOpen() override;
	void OnAssetDirty() override;

protected:
	Shader TerrainShader = { 0 };

	int SunVectorLoc = 0;
	float SunVector[3] = { 1,1,1 };
	int SpecularValueLoc = 0;
	float SpecularValue = 0;

	Model* CurrentMesh = nullptr;

	Model Cube = { 0 };
	Model Sphere = { 0 };
	Model Matbal = { 0 };
	Model Heightmap = { 0 };
	Material RenderMat = { 0 };
};