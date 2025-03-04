#include "tile_renderer.h"

#include "rlgl.h"
#include "raymath.h"

#include "external/glad.h"

constexpr float WHITEF[4] = { 1,1,1,1 };

TerainRenderer::TerainRenderer()
{
    TerrainShader = LoadShader(nullptr, nullptr);
}

void TerainRenderer::SetShader(Shader& shader)
{
    TerrainShader = shader;

    for (int i = 0; i < 5; i++)
    {
        MaterialTextureLocs.push_back(GetShaderLocation(shader, TextFormat("matDiffuse%d", i)));
        MaterialTintLocs.push_back(GetShaderLocation(shader, TextFormat("matTint%d", i)));
    }

    MaterialCountLoc = GetShaderLocation(shader, "materialCount");

    SplatmapLoc = GetShaderLocation(shader, "splatmap");
}

// Draw vertex array elements
void rlDrawVertexArrayElementsQuads(int offset, int count, const void* buffer)
{
    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned short* bufferPtr = (unsigned short*)buffer;
    if (offset > 0) bufferPtr += offset;

    glDrawElements(GL_QUADS, count, GL_UNSIGNED_SHORT, (const unsigned short*)bufferPtr);
}

void TerainRenderer::Draw(TerrainTile& tile, size_t lod)
{
    rlEnableShader(TerrainShader.id);
    rlSetUniform(TerrainShader.locs[SHADER_LOC_COLOR_DIFFUSE], WHITEF, SHADER_UNIFORM_VEC4, 1);
    rlSetUniform(TerrainShader.locs[SHADER_LOC_COLOR_SPECULAR], WHITEF, SHADER_UNIFORM_VEC4, 1);

    Matrix transform = MatrixTranslate(tile.Origin.X * tile.Info.TerrainTileSize, tile.Origin.Y * tile.Info.TerrainTileSize, 0);

    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (TerrainShader.locs[SHADER_LOC_MATRIX_VIEW] != -1)
        rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (TerrainShader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1)
        rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Model transformation matrix is send to shader uniform location: SHADER_LOC_MATRIX_MODEL
    if (TerrainShader.locs[SHADER_LOC_MATRIX_MODEL] != -1)
        rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_MODEL], transform);

    // Accumulate several model transformations:
    //    transform: model transformation provided (includes DrawModel() params combined with model.transform)
    //    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
    matModel = MatrixMultiply(transform, rlGetMatrixTransform());

    // Get model-view matrix
    matModelView = MatrixMultiply(matModel, matView);

    // Upload model normal matrix (if locations available)
    if (TerrainShader.locs[SHADER_LOC_MATRIX_NORMAL] != -1)
        rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));

    // Select current shader texture slot
    for (int i = 0; i < tile.LayerMaterials.size(); i++)
    {
        rlActiveTextureSlot(i);

        rlEnableTexture(tile.LayerMaterials[i]->DiffuseMap.id);
        rlSetUniform(MaterialTextureLocs[i], &i, SHADER_UNIFORM_INT, 1);

        float colors[4] = { float(tile.LayerMaterials[i]->DiffuseColor.r / 255.0f),
                            float(tile.LayerMaterials[i]->DiffuseColor.g / 255.0f),
                            float(tile.LayerMaterials[i]->DiffuseColor.b / 255.0f),
                            float(tile.LayerMaterials[i]->DiffuseColor.a / 255.0f) };

        rlSetUniform(MaterialTintLocs[i], colors, SHADER_UNIFORM_VEC4,1);
    }

    int maskSlot = int(tile.LayerMaterials.size());
    rlActiveTextureSlot(maskSlot);

    rlEnableTexture(tile.Splatmap.id);
    rlSetUniform(SplatmapLoc, &maskSlot, SHADER_UNIFORM_INT, 1);


    int matCount = int(tile.LayerMaterials.size());
    rlSetUniform(MaterialCountLoc, &matCount, SHADER_UNIFORM_INT, 1);

    // bind vao
    rlEnableVertexArray(tile.VaoId);

    Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

    // Send combined model-view-projection matrix to shader
    rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

    // Draw mesh
    rlDrawVertexArrayElements((int)tile.LODs[lod].IndexStart * 3, (int)tile.LODs[lod].IndexCount * 3, 0);

    // disable texture units

    int slot = 0;
    rlActiveTextureSlot(slot);
    rlDisableTexture();

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();
}
