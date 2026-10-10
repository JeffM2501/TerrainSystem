#pragma once

#include "TerrainTile.h"
#include "raylib.h"
#include <vector>
#include <span>

class TileMeshBuilder
{
public:
    void Build(TerrainTile& tile);

protected:
    Vector3 GetSiblingNormalSum(TerrainTile& tile, int16_t h, int16_t v);
    Vector3 ComputeNormalForLocation(TerrainTile& tile, int16_t h, int16_t v);
};