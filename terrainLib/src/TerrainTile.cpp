#include "TerrainTile.h"

#include "raylib.h"
#include "rlgl.h"
#include "config.h"

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

void TerrainTile::UnloadGeometry()
{
    rlUnloadVertexArray(VaoId);

    if (VboId != nullptr)
    {
        for (int i = 0; i < MAX_MESH_VERTEX_BUFFERS; i++)
            rlUnloadVertexBuffer(VboId[i]);
    }
    MemFree(VboId);

    VaoId = -1;
    VboId = nullptr;
}

void TerrainTile::UnloadSplats()
{
    UnloadTexture(Splatmap);
    Splatmap.id = -1;
}