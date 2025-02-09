#include "terrain_tile.h"

float TerrainTile::GetLocalHeight(int x, int y) const
{
    float z = GetImageColor(TerranHeightmap, x+1, y+1).r / 255.0f;
    z *= (Info.TerrainMaxZ - Info.TerrainMinZ);
    z += Info.TerrainMinZ;

    return z;
}