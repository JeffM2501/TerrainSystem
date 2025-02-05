#include "tile_renderer.h"

#include "rlgl.h"
#include "raymath.h"

constexpr float WHITEF[4] = { 1,1,1,1 };

TerainRenderer::TerainRenderer()
{
    TerrainShader = LoadShader(nullptr, nullptr);
}

void TerainRenderer::Draw(TerrainTile& tile)
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
    int slot = 0;
    rlActiveTextureSlot(slot);
    rlEnableTexture(tile.LayerMaterials[0]->DiffuseMap.id);
    rlSetUniform(TerrainShader.locs[SHADER_LOC_MAP_DIFFUSE + 0], &slot, SHADER_UNIFORM_INT, 1);

    // bind vao
    if (!rlEnableVertexArray(tile.VaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(tile.VboId[0]);
        rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(tile.VboId[1]);
        rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (TerrainShader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(tile.VboId[2]);
            rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (TerrainShader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (tile.VboId[3] != 0)
            {
                rlEnableVertexBuffer(tile.VboId[3]);
                rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for defined vertex attribute in shader but not provided by mesh
                // WARNING: It could result in GPU undefined behaviour
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(TerrainShader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
                rlDisableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (TerrainShader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(tile.VboId[4]);
            rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (TerrainShader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(tile.VboId[5]);
            rlSetVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(TerrainShader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

        rlEnableVertexBufferElement(tile.VboId[6]);

        Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(TerrainShader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        rlDrawVertexArrayElements(0, tile.Info.TerrainGridSize * tile.Info.TerrainGridSize * 3, 0);

        // disable texture units

        slot = 0;
        rlActiveTextureSlot(slot);
        rlDisableTexture();

        // Disable all possible vertex array objects (or VBOs)
        rlDisableVertexArray();
        rlDisableVertexBuffer();
        rlDisableVertexBufferElement();

        // Disable shader program
        rlDisableShader();
    }
}
