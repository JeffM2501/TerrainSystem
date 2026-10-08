#include "tile_builder.h"

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "config.h"

#include "external/glad.h"

int IndexList = -1;

TerrainLODTriangleInfo LODInfos[MaxLODLevels * MaxTransitionMasks];

void BuildLODIndexList(uint16_t* indexes, size_t& triangleIndex, int grid, int lod, uint8_t transitionMask)
{
    auto GetVertIndex = [grid](int vx, int vy) -> uint16_t {
        return (uint16_t)(vy * (grid + 1) + vx);
    };

    auto AddTriangle = [indexes, &triangleIndex](uint16_t i0, uint16_t i1, uint16_t i2) {
        indexes[(triangleIndex * 3) + 0] = i0;
        indexes[(triangleIndex * 3) + 1] = i1;
        indexes[(triangleIndex * 3) + 2] = i2;
        triangleIndex++;
    };

    int step = 1 << lod;

    // For the coarsest LOD (LOD 3, step 8), no coarser transition is possible,
    // so we generate regular quads with alternating diagonals.
    if (lod >= MaxLODLevels - 1)
    {
        bool flip = false;
        for (int y = 0; y < grid; y += step)
        {
            for (int x = 0; x < grid; x += step)
            {
                int x2 = x + step;
                int y2 = y + step;

                uint16_t p = GetVertIndex(x, y);
                uint16_t a = GetVertIndex(x2, y);
                uint16_t b = GetVertIndex(x, y2);
                uint16_t c = GetVertIndex(x2, y2);

                if (flip)
                {
                    AddTriangle(p, a, c);
                    AddTriangle(p, c, b);
                }
                else
                {
                    AddTriangle(p, a, b);
                    AddTriangle(a, c, b);
                }
                flip = !flip;
            }
            flip = !flip;
        }
        return;
    }

    // For LOD 0, 1, 2, we subdivide into macro-blocks of size 2 * step (2S x 2S).
    int macroStep = step * 2;

    for (int y = 0; y < grid; y += macroStep)
    {
        for (int x = 0; x < grid; x += macroStep)
        {
            int x0 = x;
            int x1 = x + step;
            int x2 = x + macroStep;

            int y0 = y;
            int y1 = y + step;
            int y2 = y + macroStep;

            uint16_t v00 = GetVertIndex(x0, y0);
            uint16_t v10 = GetVertIndex(x1, y0);
            uint16_t v20 = GetVertIndex(x2, y0);

            uint16_t v01 = GetVertIndex(x0, y1);
            uint16_t v11 = GetVertIndex(x1, y1); // Center vertex
            uint16_t v21 = GetVertIndex(x2, y1);

            uint16_t v02 = GetVertIndex(x0, y2);
            uint16_t v12 = GetVertIndex(x1, y2);
            uint16_t v22 = GetVertIndex(x2, y2);

            bool dropBottom = (y == 0) && (transitionMask & TRANSITION_SOUTH);
            bool dropTop    = (y2 == grid) && (transitionMask & TRANSITION_NORTH);
            bool dropLeft   = (x == 0) && (transitionMask & TRANSITION_WEST);
            bool dropRight  = (x2 == grid) && (transitionMask & TRANSITION_EAST);

            // South Quadrant (Bottom)
            if (dropBottom)
            {
                AddTriangle(v00, v20, v11);
            }
            else
            {
                AddTriangle(v00, v10, v11);
                AddTriangle(v10, v20, v11);
            }

            // East Quadrant (Right)
            if (dropRight)
            {
                AddTriangle(v20, v22, v11);
            }
            else
            {
                AddTriangle(v20, v21, v11);
                AddTriangle(v21, v22, v11);
            }

            // North Quadrant (Top)
            if (dropTop)
            {
                AddTriangle(v22, v02, v11);
            }
            else
            {
                AddTriangle(v22, v12, v11);
                AddTriangle(v12, v02, v11);
            }

            // West Quadrant (Left)
            if (dropLeft)
            {
                AddTriangle(v02, v00, v11);
            }
            else
            {
                AddTriangle(v02, v01, v11);
                AddTriangle(v01, v00, v11);
            }
        }
    }
}

void SetupIndexes(TerrainTile& tile)
{
    if (IndexList >= 0)
        return;

    int grid = tile.Info.TerrainGridSize;

    // Allocate memory for all LODs across all 16 transition masks:
    // LOD 0: 64*64*8 = 32768 per mask * 16 = 524,288 triangles
    // LOD 1: 32*32*8 = 8192 per mask * 16 = 131,072 triangles
    // LOD 2: 16*16*8 = 2048 per mask * 16 = 32,768 triangles
    // LOD 3: 512 triangles
    // Total <= 700,000 triangles
    size_t maxTriangles = (size_t)(32768 + 8192 + 2048) * MaxTransitionMasks + 1024;
    uint16_t* indexes = (uint16_t*)MemAlloc((unsigned int)(sizeof(uint16_t) * maxTriangles * 3));

    size_t triangleIndex = 0;

    for (int lod = 0; lod < MaxLODLevels - 1; lod++)
    {
        for (uint8_t mask = 0; mask < MaxTransitionMasks; mask++)
        {
            size_t entry = lod * MaxTransitionMasks + mask;
            LODInfos[entry].IndexStart = triangleIndex;
            BuildLODIndexList(indexes, triangleIndex, grid, lod, mask);
            LODInfos[entry].IndexCount = triangleIndex - LODInfos[entry].IndexStart;
        }
    }

    // For the coarsest LOD (LOD 3), generate once and share across all 16 masks
    size_t lod3Start = triangleIndex;
    BuildLODIndexList(indexes, triangleIndex, grid, MaxLODLevels - 1, TRANSITION_NONE);
    size_t lod3Count = triangleIndex - lod3Start;

    for (uint8_t mask = 0; mask < MaxTransitionMasks; mask++)
    {
        size_t entry = (MaxLODLevels - 1) * MaxTransitionMasks + mask;
        LODInfos[entry].IndexStart = lod3Start;
        LODInfos[entry].IndexCount = lod3Count;
    }

    IndexList = rlLoadVertexBufferElement(indexes, (int)(triangleIndex * 3 * sizeof(unsigned short)), false);

    MemFree(indexes);
}

std::vector<Vector3> TileMeshBuilder::GetSiblingNormals(TerrainTile& tile, int16_t h, int16_t v)
{
    std::vector<Vector3> tempNormals;

    Vector3 P = { 0,0,0 };

    float thisH = tile.GetLocalHeight(h, v);
    /*
            B
        A	P	C
            D
    */

    Vector3 A = { -1, 0, tile.GetLocalHeight(h - 1,v) - thisH };
    Vector3 B = { 0	, 1, tile.GetLocalHeight(h,v + 1) - thisH };
    Vector3 C = { 1	, 0, tile.GetLocalHeight(h + 1,v) - thisH };
    Vector3 D = { 0, -1, tile.GetLocalHeight(h,v - 1) - thisH };

    Vector3 PA = Vector3Normalize(Vector3Subtract(A, P));
    Vector3 PB = Vector3Normalize(Vector3Subtract(B, P));
    Vector3 PC = Vector3Normalize(Vector3Subtract(C, P));
    Vector3 PD = Vector3Normalize(Vector3Subtract(D, P));

    tempNormals.push_back(Vector3CrossProduct(PB, PA));
    tempNormals.push_back(Vector3CrossProduct(PC, PB));
    tempNormals.push_back(Vector3CrossProduct(PD, PC));
    tempNormals.push_back(Vector3CrossProduct(PA, PD));

    return tempNormals;
}

Vector3 TileMeshBuilder::ComputeNormalForLocation(TerrainTile& tile, int16_t h, int16_t v)
{
    std::vector<Vector3> tempNormals = GetSiblingNormals(tile, h, v);

    Vector3 totalNormal = { 0,0,0 };
    for (const auto& norm : tempNormals)
    {
        totalNormal = Vector3Add(norm, totalNormal);
    }

    totalNormal = Vector3Scale(totalNormal, 1.0f / tempNormals.size());

    return totalNormal;
}
constexpr int MAX_MESH_VERTEX_BUFFERS = 9;
void TileMeshBuilder::Build(TerrainTile& tile)
{
    // upload the buffers
    tile.VboId = (unsigned int*)MemAlloc(MAX_MESH_VERTEX_BUFFERS * sizeof(unsigned int));

    tile.VaoId = 0;        // Vertex Array Object
    tile.VboId[0] = 0;     // Vertex buffer: positions
    tile.VboId[1] = 0;     // Vertex buffer: texcoords
    tile.VboId[2] = 0;     // Vertex buffer: normals
    tile.VboId[3] = 0;     // Vertex buffer: colors
    tile.VboId[4] = 0;     // Vertex buffer: tangents
    tile.VboId[5] = 0;     // Vertex buffer: texcoords2
    tile.VboId[6] = 0;     // Vertex buffer: indices

    SetupIndexes(tile);

    uint32_t vertCount = uint32_t(tile.Info.TerrainGridSize + 1) * uint32_t(tile.Info.TerrainGridSize + 1);
    uint32_t triangleCount = uint32_t(tile.Info.TerrainGridSize) * uint32_t(tile.Info.TerrainGridSize) * 2;

    float* verts = (float*)MemAlloc(sizeof(float) * vertCount * 3);
    float* normals = (float*)MemAlloc(sizeof(float) * vertCount * 3);
    float* textureCords = (float*)MemAlloc(sizeof(float) * vertCount * 2);
    float* textureCord2s = (float*)MemAlloc(sizeof(float) * vertCount * 2);
    float* tangents = nullptr;

    uint8_t* colors = (uint8_t*)MemAlloc(vertCount * 4);
 
    float vertexScale = tile.Info.TerrainTileSize / tile.Info.TerrainGridSize;
    float uv2Scale = tile.Info.TerrainTileSize / (tile.Info.TerrainGridSize * 4);
    
    // generate the attribute buffers
    int vertIndex = 0;
    for (int y = 0; y < tile.Info.TerrainGridSize + 1; y++)
    {
        for (int x = 0; x < tile.Info.TerrainGridSize + 1; x++)
        {
            float z = tile.GetLocalHeight(x, y);

            verts[(vertIndex * 3) + 0] = x * vertexScale;
            verts[(vertIndex * 3) + 1] = y * vertexScale;
            verts[(vertIndex * 3) + 2] = z;

            auto normal = ComputeNormalForLocation(tile, x, y);
            normals[(vertIndex * 3) + 0] = normal.x;
            normals[(vertIndex * 3) + 1] = normal.y;
            normals[(vertIndex * 3) + 2] = normal.z;

            textureCords[(vertIndex * 2) + 0] = x / (float)(tile.Info.TerrainGridSize + 1);
            textureCords[(vertIndex * 2) + 1] = y / (float)(tile.Info.TerrainGridSize + 1);

            textureCord2s[(vertIndex * 2) + 0] = x * uv2Scale;
            textureCord2s[(vertIndex * 2) + 1] = y * uv2Scale;

            colors[(vertIndex * 4) + 0] = 255;
            colors[(vertIndex * 4) + 1] = 255;
            colors[(vertIndex * 4) + 2] = 255;
            colors[(vertIndex * 4) + 3] = 255;

            vertIndex++;
        }
    }

    tile.VaoId = rlLoadVertexArray();
    rlEnableVertexArray(tile.VaoId);

    tile.VboId[0] = rlLoadVertexBuffer(verts, vertCount * 3 * sizeof(float), false);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);

    tile.VboId[1] = rlLoadVertexBuffer(textureCords, vertCount * 2 * sizeof(float), false);
    rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(1);

    tile.VboId[2] = rlLoadVertexBuffer(normals, vertCount * 3 * sizeof(float), false);
    rlSetVertexAttribute(2, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(2);
   
    tile.VboId[3] = rlLoadVertexBuffer(colors, vertCount * 4 * sizeof(unsigned char), false);
    rlSetVertexAttribute(3, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
    rlEnableVertexAttribute(3);

    if (tangents)
    {
        tile.VboId[4] = rlLoadVertexBuffer(tangents, vertCount * 4 * sizeof(float), false);
        rlSetVertexAttribute(4, 4, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(4);
    }
    else
    {
        // Default vertex attribute: tangent
        // WARNING: Default value provided to shader if location available
        float value[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        rlSetVertexAttributeDefault(4, value, SHADER_ATTRIB_VEC4, 4);
        rlDisableVertexAttribute(4);
    }

    tile.VboId[5] = rlLoadVertexBuffer(textureCord2s, vertCount * 2 * sizeof(float), false);
    rlSetVertexAttribute(5, 2, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(5);
  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexList);
    tile.VboId[6] = IndexList;
    tile.LODs = LODInfos;

    rlDisableVertexArray();

    MemFree(colors);
    MemFree(textureCord2s);
    MemFree(textureCords);
    MemFree(normals);
    MemFree(verts);
}