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
    Color   DiffuseColor = WHITE;
    Texture NormalMap;
};

struct TerrainInfo
{
    uint8_t TerrainGridSize = 128;

    float TerrainTileSize = 128;
    float TerrainMinZ = 0;
    float TerrainMaxZ = 300;
};

static constexpr uint8_t MaxLODLevels = 4;
static constexpr uint8_t MaxTransitionMasks = 16;

enum TerrainTransitionEdge : uint8_t
{
    TRANSITION_NONE  = 0,
    TRANSITION_NORTH = 1 << 0, // +Y edge (y = GridSize)
    TRANSITION_SOUTH = 1 << 1, // -Y edge (y = 0)
    TRANSITION_EAST  = 1 << 2, // +X edge (x = GridSize)
    TRANSITION_WEST  = 1 << 3, // -X edge (x = 0)
};

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

    std::vector<TerrainMaterial*> LayerMaterials;
    Texture Splatmap;

    unsigned int VaoId = -1;
    unsigned int* VboId = nullptr;

    const TerrainLODTriangleInfo* LODs = nullptr;

    TerrainTile(TerrainInfo& info);
    ~TerrainTile();

    void SetHeightsFromImage(Image& image);

    float GetLocalHeight(int x, int y) const;

    void Unload();
};