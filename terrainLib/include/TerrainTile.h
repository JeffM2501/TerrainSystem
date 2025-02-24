#pragma once

#include "raylib.h"

#include <stdint.h>
#include <vector>

struct TerrainPosition
{
    int64_t X = 0;
    int64_t Y = 0;

    bool operator == (const TerrainPosition& other) const
    {
        return X == other.X && Y == other.Y;
    }   
};

struct TerrainMaterial
{
    Texture DiffuseMap;
    Color   DiffuseColor = WHITE;
    Texture NormalMap;
};

struct TerrainInfo
{
    uint8_t TerrainGridSize = 128;

    float TerrainTileSize = 128;
    float TerrainMinZ = -50;
    float TerrainMaxZ = 100;
};

static constexpr uint8_t MaxLODLevels = 4;

struct TerrainLODTriangleInfo
{
    size_t IndexStart = -1;
    size_t IndexCount = 0;
};

struct TerrainTile
{
    TerrainInfo& Info;

    TerrainPosition Origin = { 0,0 };

    std::vector<float> TerrainHeightMap;

    std::vector<const TerrainMaterial*> LayerMaterials;
    Texture Splatmap;

    unsigned int VaoId = -1;
    unsigned int* VboId = nullptr;

    const TerrainLODTriangleInfo* LODs = nullptr;

    TerrainTile(TerrainInfo& info);
    ~TerrainTile();

    void SetHeightsFromImage(Image& image);

    void AddMaterial(const TerrainMaterial* material);

    float GetLocalHeight(int x, int y) const;

    void UnloadGeometry();
    void UnloadSplats();
};