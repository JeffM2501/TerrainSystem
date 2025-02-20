#include "TerrainTile.h"

TerrainTile::TerrainTile(TerrainInfo& info)
    : Info(info)
{

}

TerrainTile::~TerrainTile()
{
}

void TerrainTile::SetHeightsFromImage(Image& image)
{
    TerrainHeightMap.resize((Info.TerrainGridSize + 3) * (Info.TerrainGridSize + 3));

    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            size_t index = (y * (Info.TerrainGridSize + 3)) + x;
            float z = GetImageColor(image, x, y).r / 255.0f;
            z *= (Info.TerrainMaxZ - Info.TerrainMinZ);
            z += Info.TerrainMinZ;

            TerrainHeightMap[index] = z;
        }
    }
}

float TerrainTile::GetLocalHeight(int x, int y) const
{
    size_t index = (y + 1) * (Info.TerrainGridSize + 3) + x + 1;
    return TerrainHeightMap[index];
}