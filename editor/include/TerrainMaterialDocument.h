#pragma once

#include "Document.h"

#include "TerrainTile.h"
#include "LifetimeToken.h"
#include "AssetEditManager.h"

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

	~TerrainMaterialDocument();
protected:
	void OnShowScene(const Vector2& renderSize) override;

	void RegisterEditHandler();
	void RebuildMaterial();

	void DrawMeshExample(Mesh& mesh);

protected:
	AssetTypes::TerrainMaterialAsset* AssetData = nullptr;
	Tokens::TokenSource Token;

	AssetEditManager EditManager;

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