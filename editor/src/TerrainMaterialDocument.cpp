#include "TerrainMaterialDocument.h"

#include "PropertiesPanel.h"
#include "EditorThumbnailManager.h"
#include "AssetManager.h"
#include "Application.h"
#include "Camera.h"
#include "extras/IconsFontAwesome6.h"

#include "rlgl.h"

using namespace AssetSystem;
using namespace EditorFramework;

TerrainMaterialDocument::~TerrainMaterialDocument()
{
    if (IsModelValid(Cube))
        UnloadMesh(Cube.meshes[0]);

    if (IsModelValid(Sphere))
        UnloadMesh(Sphere.meshes[0]);

    if (IsModelValid(Heightmap))
        UnloadMesh(Heightmap.meshes[0]);
}

std::string_view TerrainMaterialDocument::GetDocumentName()
{
    if (AssetData && !AssetData->GetPath().empty())
        return GetFileNameWithoutExt(AssetData->GetPath().c_str());

    static std::string unknownName;
    unknownName = "Untitled ";
    unknownName += AssetTypes::TerrainMaterialAsset::TypeName;

    return unknownName;
}

void TerrainMaterialDocument::CreateAsset()
{
    AssetData = EditManager.CreateTempAsset<AssetTypes::TerrainMaterialAsset>();
    RegisterEditHandler();
}

void TerrainMaterialDocument::OpenAsset(const std::string& assetPath)
{
    AssetData = EditManager.OpenAsset<AssetTypes::TerrainMaterialAsset>(AssetManager::ToAssetPath(assetPath));
    OnActivated();
    RegisterEditHandler();
    RebuildMaterial();
}

void TerrainMaterialDocument::RegisterEditHandler()
{
    AssetData->ValuePtr->OnValueChanged.Add([this](const Types::ValueChangedEvent& event)
        {
            SetDirty();
            RebuildMaterial();
        }, Token.GetToken());
}

void TerrainMaterialDocument::SaveAsset()
{
    if (!AssetData)
        return;
    EditManager.SaveAsset(AssetData);
    SetClean();
}

void TerrainMaterialDocument::SaveAsAsset(const std::string& assetPath)
{
    if (!AssetData)
        return;
    EditManager.SaveAssetAs(AssetData, AssetManager::ToAssetPath(assetPath));
    SetClean();
}

void TerrainMaterialDocument::OnShowScene(const Vector2& renderSize)
{
    SetShaderValue(TerrainShader, SunVectorLoc, SunVector, SHADER_UNIFORM_VEC3);

    float cameraPos[3] = { VieportCamera.GetCamera()->position.x, VieportCamera.GetCamera()->position.y, VieportCamera.GetCamera()->position.z };
    SetShaderValue(TerrainShader, TerrainShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    SetShaderValue(TerrainShader, SpecularValueLoc, &SpecularValue, SHADER_UNIFORM_FLOAT);

    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    DrawGrid(8, 0.25f);
    rlPopMatrix();

    if (!AssetData || !CurrentMesh)
        return;

    CurrentMesh->materials[0] = RenderMat;
    DrawModel(*CurrentMesh, Vector3Zeros, 1, WHITE);
}

void TerrainMaterialDocument::OnCreated()
{
    ClearColor = DARKGRAY;

    VieportCamera.TranslateSpeed = 5;

    VieportCamera.FindController<FPSCameraController>()->EnabledState = EditorCameraController::EnableState::Disabled;
    VieportCamera.FindController<FocusCameraController>()->EnabledState = EditorCameraController::EnableState::Disabled;
    VieportCamera.FindController<OribitCameraController>()->EnabledState = EditorCameraController::EnableState::EnabledAlways;

    VieportCamera.GetCamera()->position = { 0, -5, 3 };
    VieportCamera.GetCamera()->target.z = 0.5f;

    TerrainShader = LoadShader("resources/shaders/material_preview.vs", "resources/shaders/material_preview.fs");
    if (!IsShaderValid(TerrainShader))
        TerrainShader = LoadShader(nullptr, nullptr);

    SunVectorLoc = GetShaderLocation(TerrainShader, "sunVector");

    NearPlane = 0.1f;
    FarPlane = 5000.0f;

    Cube = LoadModelFromMesh(GenMeshCube(1, 1, 1));
    Cube.transform = MatrixTranslate(0, 0, 0.5f);

    Sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32));
    Sphere.transform = MatrixTranslate(0, 0, 0.5f);

    Image heightmap = GenImagePerlinNoise(128, 128, 3, 4, 1.5f);
    Heightmap = LoadModelFromMesh(GenMeshHeightmap(heightmap, Vector3{ 2,1,2 }));
    UnloadImage(heightmap);
    Heightmap.transform = MatrixMultiply(MatrixRotateZYX(Vector3{ DEG2RAD * 90, 0, 0 }), MatrixTranslate(-1, 1, 0));

    Matbal = LoadModel("resources/meshes/export3dcoat.obj");

    float scale = 0.075f;
    Matbal.transform = MatrixMultiply(MatrixTranslate(0, 7.65f, 0), MatrixMultiply(MatrixRotateX(DEG2RAD * 90), MatrixScale(scale, scale, scale)));

    CurrentMesh = &Cube;

    RenderMat = LoadMaterialDefault();
    RenderMat.shader = TerrainShader;

    auto lightingGroup = MainToolbar.AddGroup("Lighting");
    auto specularCommand = lightingGroup->AddItem<StateMenuCommand>(0, ICON_FA_LIGHTBULB, "Specular",
        [this](CommandContextSet*)
        {
            if (SpecularValue <= 0)
                SpecularValue = 16;
            else
                SpecularValue = 0;

            TraceLog(LOG_INFO, "Specular Level = %f", SpecularValue);
        },
        [this](CommandContextSet*)
        {
            return SpecularValue > 0;
        });

    auto shapeGroup = MainToolbar.AddGroup("Shapes");
    shapeGroup->AddItem<StateMenuCommand>(0, ICON_FA_CUBE, "Cube",
        [this](CommandContextSet*)
        {
            CurrentMesh = &Cube;
        },
        [this](CommandContextSet*)
        {
            return CurrentMesh == &Cube;
        });

    shapeGroup->AddItem<StateMenuCommand>(0, ICON_FA_GLOBE, "Globe",
        [this](CommandContextSet*)
        {
            CurrentMesh = &Sphere;
        },
        [this](CommandContextSet*)
        {
            return CurrentMesh == &Sphere;
        });

    shapeGroup->AddItem<StateMenuCommand>(0, ICON_FA_GOLF_BALL_TEE, "Matbal",
        [this](CommandContextSet*)
        {
            CurrentMesh = &Matbal;
        },
        [this](CommandContextSet*)
        {
            return CurrentMesh == &Matbal;
        });

    shapeGroup->AddItem<StateMenuCommand>(0, ICON_FA_MOUNTAIN, "Terrain",
        [this](CommandContextSet*)
        {
            CurrentMesh = &Heightmap;
        },
        [this](CommandContextSet*)
        {
            return CurrentMesh == &Heightmap;
        });
}

void TerrainMaterialDocument::RebuildMaterial()
{
    if (!AssetData || AssetData->GetMaterial().GetDiffuseTexture().GetPath().empty())
        RenderMat.maps[MATERIAL_MAP_ALBEDO].texture = Texture2D{ rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    else
        RenderMat.maps[MATERIAL_MAP_ALBEDO].texture = Editor::ThumbnailManager::GetThumbnail(AssetData->GetMaterial().GetDiffuseTexture().GetPath());

    if (AssetData && AssetData->GetMaterial().GetMaterialType() == AssetTypes::TerrainMaterialTypeEnum::Values::Tintable)
        RenderMat.maps[MATERIAL_MAP_ALBEDO].color = AssetData->GetMaterial().GetDiffuseColor();
    else
        RenderMat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
}

void TerrainMaterialDocument::OnActivated()
{
    auto* properties = GetApp()->GetPanel<PropertiesPanel>();
    if (properties && AssetData)
        properties->SetObject(AssetData->ValuePtr);

    EditManager.Activate();
}

void TerrainMaterialDocument::OnDeactivated()
{
    auto* properties = GetApp()->GetPanel<PropertiesPanel>();
    if (properties)
        properties->SetObject(nullptr);

    EditManager.Deactivate();
}