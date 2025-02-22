#pragma once

#include "raylib.h"

#include "TerrainTile.h"

#include <vector>
#include <unordered_map>

class TerainRenderer
{
protected:
    Shader TerrainShader = { 0 };

    std::vector<int> MaterialTextureLocs;
    std::vector<int> MaterialTintLocs;

    int MaterialCountLoc = -1;

    int SplatmapLoc = -1;

public:
    std::unordered_map<size_t, TerrainMaterial> MaterialLibrary;

    // materials?

    TerainRenderer();

    void SetShader(Shader& shader);

    void Draw(TerrainTile& tile, size_t lod = 0);
};