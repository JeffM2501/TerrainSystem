#pragma once

#include "TerrainTile.h"
#include "raylib.h"
#include <vector>

class TileMeshBuilder
{
public:
    void Build(TerrainTile& tile);

protected:
    std::vector<Vector3> GetSiblingNormals(TerrainTile& tile, int16_t h, int16_t v);
    Vector3 TileMeshBuilder::ComputeNormalForLocation(TerrainTile& tile, int16_t h, int16_t v);
};