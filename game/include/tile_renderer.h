#pragma once

#include "terrain_tile.h"

#include "raylib.h"

class TerainRenderer
{
public:
    Shader TerrainShader = { 0 };

    // materials?

    TerainRenderer();

    void Draw(TerrainTile& tile);
};