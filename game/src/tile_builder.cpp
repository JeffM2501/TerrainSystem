#include "tile_builder.h"

#include "raylib.h"
#include "rlgl.h"
#include "config.h"

void TileMeshBuilder::Build(TerrainTile& tile)
{
    uint32_t vertCount = (tile.Info.TerrainGridSize + 1) * (tile.Info.TerrainGridSize + 1);
    uint32_t triangleCount = tile.Info.TerrainGridSize * tile.Info.TerrainGridSize * 2;

    float* verts = (float*)MemAlloc(sizeof(float) * vertCount * 3);
    float* normals = (float*)MemAlloc(sizeof(float) * vertCount * 3);
    float* textureCords = (float*)MemAlloc(sizeof(float) * vertCount * 2);
    float* textureCord2s = (float*)MemAlloc(sizeof(float) * vertCount * 2);
    float* tangents = nullptr;

    uint8_t* colors = (uint8_t*)MemAlloc(vertCount * 4);

    uint16_t* indexes = (uint16_t*)MemAlloc(sizeof(uint16_t) * triangleCount * 3);

    float vertexScale = tile.Info.TerrainTileSize / tile.Info.TerrainGridSize;
    float uv2Scale = tile.Info.TerrainTileSize / tile.Info.TerrainGridSize;
    
    // generate the attribute buffers
    int vertIndex = 0;
    for (int y = 0; y < tile.Info.TerrainGridSize + 1; y++)
    {
        for (int x = 0; x < tile.Info.TerrainGridSize + 1; x++)
        {
            float z = GetImageColor(tile.TerranHeightmap, x, y).r / 255.0f;
            z *= (tile.Info.TerrainMaxZ - tile.Info.TerrainMinZ);
            z += tile.Info.TerrainMinZ;

            verts[(vertIndex * 3) + 0] = x * vertexScale;
            verts[(vertIndex * 3) + 1] = y * vertexScale;
            verts[(vertIndex * 3) + 2] = z;

            normals[(vertIndex * 3) + 0] = 0;
            normals[(vertIndex * 3) + 1] = 0;
            normals[(vertIndex * 3) + 2] = 1;

            textureCords[(vertIndex * 2) + 0] = x / (float)(tile.Info.TerrainGridSize + 1);
            textureCords[(vertIndex * 2) + 1] = y / (float)(tile.Info.TerrainGridSize + 1);

            textureCord2s[(vertIndex * 2) + 0] = x * uv2Scale;
            textureCord2s[(vertIndex * 2) + 1] = y * uv2Scale;

            colors[(vertIndex * 4) + 0] = x;
            colors[(vertIndex * 4) + 1] = y;
            colors[(vertIndex * 4) + 2] = 255;
            colors[(vertIndex * 4) + 3] = 255;

            vertIndex++;
        }
    }

    uint32_t triangleIndex = 0;
    bool flip = false;
    // generate the index list
    for (int y = 0; y < tile.Info.TerrainGridSize; y++)
    {
        for (int x = 0; x < tile.Info.TerrainGridSize; x++)
        {
            int x2 = x + 1;
            int y2 = y + 1;

            /*
                B	C

                P	A
            */
            uint16_t p = y * (tile.Info.TerrainGridSize + 1) + x;
            uint16_t a = y * (tile.Info.TerrainGridSize + 1) + x2;
            uint16_t b = y2 * (tile.Info.TerrainGridSize + 1) + x;
            uint16_t c = y2 * (tile.Info.TerrainGridSize + 1) + x2;

            if (flip)
            { 
                // PAC
                indexes[(triangleIndex * 3) + 0] = p;
                indexes[(triangleIndex * 3) + 1] = a;
                indexes[(triangleIndex * 3) + 2] = c;
                triangleIndex++;

                // PCB
                indexes[(triangleIndex * 3) + 0] = p;
                indexes[(triangleIndex * 3) + 1] = c;
                indexes[(triangleIndex * 3) + 2] = b;
                triangleIndex++;
            }
            else
            {
                // PAB
                indexes[(triangleIndex * 3) + 0] = p;
                indexes[(triangleIndex * 3) + 1] = a;
                indexes[(triangleIndex * 3) + 2] = b;
                triangleIndex++;
                // ACB
                indexes[(triangleIndex * 3) + 0] = a;
                indexes[(triangleIndex * 3) + 1] = c;
                indexes[(triangleIndex * 3) + 2] = b;
                triangleIndex++;
            }

            flip = !flip;
        }
    }

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
  
    tile.VboId[6] = rlLoadVertexBufferElement(indexes, triangleCount * 3 * sizeof(unsigned short), false);

    rlDisableVertexArray();

    MemFree(indexes);
    MemFree(colors);
    MemFree(textureCord2s);
    MemFree(textureCords);
    MemFree(normals);
    MemFree(verts);
}