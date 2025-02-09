#pragma once

#include "raylib.h"

#include <stdint.h>
#include <vector>

struct TerrainPosition
{
    int64_t X = 0;
    int64_t Y = 0;
};

struct TerrainMaterial
{
    Texture DiffuseMap;
    int DiffuseShaderLoc;

    Texture NormalMap;
    int NormalShaderLoc;
};

struct TerrainInfo
{
    uint8_t TerrainGridSize = 128;

    float TerrainTileSize = 128;
    float TerrainMinZ = 0;
    float TerrainMaxZ = 300;
};

static constexpr uint8_t MaxLODLevels = 4;

struct TerrainLODTriangleInfo
{
    size_t IndexStart= -1;
    size_t IndexCount = 0;
};

struct TerrainTile
{
    TerrainInfo& Info;

    TerrainPosition Origin = { 0,0 };

   std::vector<float> TerrainHeightMap;

    std::vector<TerrainMaterial*> LayerMaterials;
    std::vector<Image> LayerSplatMaps;

    unsigned int VaoId = -1;
    unsigned int* VboId = nullptr;

    const TerrainLODTriangleInfo *LODs = nullptr;

    TerrainTile(TerrainInfo& info);
    ~TerrainTile();

    void SetHeightsFromImage(Image& image);

    float GetLocalHeight(int x, int y) const;
};