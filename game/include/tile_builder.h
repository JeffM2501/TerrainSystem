#pragma once

#include "terrain_tile.h"

#include <vector>

class TileMeshBuilder
{
public:
    void Build(TerrainTile& tile);

protected:
    std::vector<Vector3> GetSiblingNormals(TerrainTile& tile, int16_t h, int16_t v);
    Vector3 ComputeNormalForLocation(TerrainTile& tile, int16_t h, int16_t v);
};