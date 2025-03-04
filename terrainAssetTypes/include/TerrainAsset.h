#pragma once

#include "AssetFile.h"
#include "AssetReference.h"
#include "TerrainMaterial.h"

#include "TerrainTile.h"
#include <vector>

namespace TerrainAssets
{
    class TerrainTileInfo
    {
    public:
        TerrainPosition     Origin = { 0,0 };
        std::vector<float>  HeightMap;
    };

    class TerrainAsset : public AssetSystem::AssetFile
    {
    public:
        REGISTER_ASSET(TerrainAsset);

        TerrainInfo Info;
        std::vector<TerrainTileInfo> TileInfos;

        std::vector<AssetSystem::AssetReference<TerrainMaterialAsset>> MaterialLibrary;
    };
}