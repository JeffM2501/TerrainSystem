#include "TerrainTile.h"

#include "raylib.h"
#include "rlgl.h"
#include "config.h"

constexpr int MAX_MESH_VERTEX_BUFFERS = 9;

TerrainTile::TerrainTile(TerrainInfo& info)
    : Info(info), MinZ(info.TerrainMinZ), MaxZ(info.TerrainMaxZ)
{
    Bounds = GetBoundingBox();
}

TerrainTile::~TerrainTile()
{
}

BoundingBox TerrainTile::GetBoundingBox() const
{
    float minX = (float)Origin.X * Info.TerrainTileSize;
    float minY = (float)Origin.Y * Info.TerrainTileSize;
    float maxX = (float)(Origin.X + 1) * Info.TerrainTileSize;
    float maxY = (float)(Origin.Y + 1) * Info.TerrainTileSize;
    return BoundingBox{ Vector3{ minX, minY, MinZ }, Vector3{ maxX, maxY, MaxZ } };
}

void TerrainTile::SetHeightsFromImage(Image& image)
{
    TerrainHeightMap.resize((Info.TerrainGridSize + 3) * (Info.TerrainGridSize + 3));

    float minZ = 1e30f;
    float maxZ = -1e30f;

    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            size_t index = (y * (Info.TerrainGridSize + 3)) + x;
            float z = GetImageColor(image, x, y).r / 255.0f;
            z *= (Info.TerrainMaxZ - Info.TerrainMinZ);
            z += Info.TerrainMinZ;

            TerrainHeightMap[index] = z;
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }

    if (minZ <= maxZ)
    {
        MinZ = minZ;
        MaxZ = maxZ;
    }
    Bounds = GetBoundingBox();
}

void TerrainTile::InitHeightMap()
{
    TerrainHeightMap.resize((Info.TerrainGridSize + 3) * (Info.TerrainGridSize + 3), 0.0f);
}

void TerrainTile::SetHeightAt(int x, int y, float param)
{
    float z = param *(Info.TerrainMaxZ - Info.TerrainMinZ);
    z += Info.TerrainMinZ;

    int maxDimension = Info.TerrainGridSize + 3;
    if (x >= 0 && x < maxDimension && y >= 0 && y < maxDimension)
    {
        size_t index = (y * maxDimension) + x;
        TerrainHeightMap[index] = z;
        if (z < MinZ) MinZ = z;
        if (z > MaxZ) MaxZ = z;
        Bounds = GetBoundingBox();
    }
}

void TerrainTile::AddMaterial(const TerrainMaterial* material)
{
    if (material)
        LayerMaterials.push_back(material);
}

#include <algorithm>
#include <cmath>


float TerrainTile::GetHeightAt(float localX, float localY) const
{
    if (TerrainHeightMap.empty() || Info.TerrainTileSize <= 0.0f)
        return 0.0f;

    float gridScale = (float)Info.TerrainGridSize / Info.TerrainTileSize;
    float gx = localX * gridScale;
    float gy = localY * gridScale;

    gx = std::clamp(gx, 0.0f, (float)Info.TerrainGridSize);
    gy = std::clamp(gy, 0.0f, (float)Info.TerrainGridSize);

    int ix = (int)std::floor(gx);
    int iy = (int)std::floor(gy);

    if (ix >= Info.TerrainGridSize)
        ix = Info.TerrainGridSize - 1;
    if (iy >= Info.TerrainGridSize)
        iy = Info.TerrainGridSize - 1;

    float fx = gx - (float)ix;
    float fy = gy - (float)iy;

    float h00 = GetLocalHeight(ix, iy);
    float h10 = GetLocalHeight(ix + 1, iy);
    float h01 = GetLocalHeight(ix, iy + 1);
    float h11 = GetLocalHeight(ix + 1, iy + 1);

    float h0 = h00 + (h10 - h00) * fx;
    float h1 = h01 + (h11 - h01) * fx;
    return h0 + (h1 - h0) * fy;
}

void TerrainTile::UnloadGeometry()
{
    rlUnloadVertexArray(VaoId);

    if (VboId != nullptr)
    {
        for (int i = 0; i < MAX_MESH_VERTEX_BUFFERS; i++)
        {
            if (i != 1 && i != 5 && i != 6)
                rlUnloadVertexBuffer(VboId[i]);
        }
            
    }
    MemFree(VboId);

    VaoId = -1;
    VboId = nullptr;

    TerrainHeightMap.clear();
}

void TerrainTile::UnloadSplats()
{
    UnloadTexture(Splatmap);
    LayerMaterials.clear();
    Splatmap.id = -1;
}