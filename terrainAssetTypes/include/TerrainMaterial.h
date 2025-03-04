#pragma once

#include "AssetFile.h"
#include "AssetReference.h"
#include <string>

namespace TerrainAssets
{
    class TerrainMaterialAsset : public AssetSystem::AssetFile
    {
    public:
        REGISTER_ASSET(TerrainMaterialAsset);

        AssetSystem::ResrouceReference DiffuseMap;
        AssetSystem::ResrouceReference NormalMap;
    };
}