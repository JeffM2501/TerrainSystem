#include "TerrainDocument.h"
#include "Application.h"
#include "DisplayScale.h"
#include "Dialog.h"

#include "extras/IconsFontAwesome6.h"

#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"
#include "rlgl.h"

static constexpr float CAMERA_MOVE_SPEED = 20;
static constexpr float CAMERA_ROTATION_SPEED = 1;
static constexpr float CAMERA_PAN_SPEED = 20;
static constexpr float CAMERA_ORBITAL_SPEED = 0.25f;
static constexpr float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.003f;

using namespace EditorFramework;

std::string_view TerrainDocument::GetDocumentName()
{
    if (AssetPath.empty())
        return TextFormat("Untitled Terrain %d", DocumentID);
    else
        return GetFileNameWithoutExt(AssetPath.c_str());
}

void TerrainDocument::OnUpdate(int width, int height)
{
    ViewportDocument::OnUpdate(width, height);

    SetShaderValue(TerrainShader, SunVectorLoc, SunVector, SHADER_UNIFORM_VEC3);
}

void TerrainDocument::OnShowScene(const Vector2& renderSize)
{
    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    DrawGrid(1024, 16);
    rlPopMatrix();

    DrawCube(Vector3Zeros, 1, 1, 1, RED);
    DrawCube(Vector3{ Info.TerrainGridSize * 1.0f, Info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
    DrawCube(Vector3{ 0, Info.TerrainGridSize * 1.0f,0 }, 1, 1, 1, BLUE);
    DrawCube(Vector3{ Info.TerrainGridSize * 1.0f,0,0 }, 1, 1, 1, BLUE);

    DrawCube(Vector3{ 0,Info.TerrainGridSize * 0.5f,0 }, 0.5f, 0.5f, 0.5f, YELLOW);
    DrawCube(Vector3{ Info.TerrainGridSize * 0.5f,0,0 }, 0.5f, 0.5f, 0.5f, YELLOW);

    DrawCube(Vector3{ 0,1,0 }, 0.125f, 2, 0.125f, PURPLE);

    int showSplat = ShowSplat ? 1 : 0;
    SetShaderValue(TerrainShader, ShowSplatFlagLoc, &showSplat, SHADER_UNIFORM_INT);

    // draw terrain
    for (int i = 0; i < Tiles.size(); i++)
    {
        // todo, compute the distance from the camera tile
        int lod = (int)std::max(Tiles[i].Origin.X, Tiles[i].Origin.Y) / 3;
        if (lod >= MaxLODLevels)
            lod = MaxLODLevels - 1;

       
        int selected = 0;
        SetShaderValue(TerrainShader, SelectedShaderFlagLoc, &selected, SHADER_UNIFORM_INT);
        Renderer.Draw(Tiles[i], lod);
        if (Tiles[i].Origin == SelectedTileLoc)
        {
            rlEnableWireMode();
            rlSetLineWidth(2);
            //rlDisableDepthTest();
            selected = 1;
            SetShaderValue(TerrainShader, SelectedShaderFlagLoc, &selected, SHADER_UNIFORM_INT);
            Renderer.Draw(Tiles[i], lod);
           // rlEnableDepthTest();
   
            rlDisableWireMode();
            rlSetLineWidth(1);
        }
    }
}

void TerrainDocument::OnShowUI()
{
    auto size = GetButtonSize(ICON_FA_BOX);

    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
    ImGui::SetNextWindowSize(ImVec2(-1,-1));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoSavedSettings;


    ImGui::Begin(ICON_FA_PALETTE "###Tools", nullptr, flags);
  //  ImGui::PopStyleVar();
    ImGui::Button(ICON_FA_PAINTBRUSH);
    ImGui::End();
}

void TerrainDocument::OnCreated()
{
    TerrainShader = LoadShader("resources/base.vs", "resources/base.fs");
    if (!IsShaderValid(TerrainShader))
        TerrainShader = LoadShader(nullptr, nullptr);

    SunVectorLoc = GetShaderLocation(TerrainShader, "sunVector");

    rlSetClipPlanes(0.1f, 5000.0f);

    float selectedColor[4] = { 1,1,0,1 };
    SetShaderValue(TerrainShader, GetShaderLocation(TerrainShader,"selectedColor"), selectedColor, SHADER_UNIFORM_VEC4);

    SelectedShaderFlagLoc = GetShaderLocation(TerrainShader, "selected");
    ShowSplatFlagLoc = GetShaderLocation(TerrainShader, "showSplat");

    Renderer.SetShader(TerrainShader);

    LoadMaterial("Grass", "resources/terrain_materials/grass_ground_d-resized.png");
    LoadMaterial("Ground", "resources/terrain_materials/ground_crackedv_d-resized.png");
    LoadMaterial("Road", "resources/terrain_materials/ground_dry_d-resized.png");
    LoadMaterial("Snow", "resources/terrain_materials/snow_grass3_d-resized.png");

    LoadMaterial("Grid", "resources/grid.png");

    auto visGroup = MainToolbar.AddGroup("TerrainVis");
    auto splatCommand = visGroup->AddItem<StateMenuCommand>(0, ICON_FA_SPLOTCH, "Show Splatmap", [this](CommandContextSet*) {ShowSplat = !ShowSplat; }, [this](CommandContextSet*) {return ShowSplat; });

    auto& cameraGroup = MainToolbar.AddGroup("Cameras");

    auto viewMenu = DocumentMenuBar.AddSubItem("View", "", 20);
    auto showGroup = viewMenu->AddGroup("Show", ICON_FA_EYE);
    showGroup->AddItem(0, splatCommand);
}

TerrainTile& TerrainDocument::GetTile(int x, int y)
{
    for (auto& tile : Tiles)
    {
        if (tile.Origin.X == x && tile.Origin.Y == y)
            return tile;
    }
    auto& tile = Tiles.emplace_back(Info);
    tile.Origin.X = x;
    tile.Origin.Y = y;

    if (x > TerrainBounds.X)
        TerrainBounds.X = x;
    if (y > TerrainBounds.Y)
        TerrainBounds.Y = y;

    return tile;
}

bool TerrainDocument::HasTile(int x, int y) const
{
    for (auto& tile : Tiles)
    {
        if (tile.Origin.X == x && tile.Origin.Y == y)
            return true;
    }

    return false;
}

void TerrainDocument::LoadMaterial(const std::string& name, std::string_view path)
{
    TerrainMaterial &mat = MaterialLibrary.insert_or_assign(name, TerrainMaterial()).first->second;
    if (mat.DiffuseMap.id > 0)
        UnloadTexture(mat.DiffuseMap);

    mat.DiffuseMap = LoadTexture(path.data());
    GenTextureMipmaps(&mat.DiffuseMap);
    SetTextureFilter(mat.DiffuseMap, TEXTURE_FILTER_TRILINEAR);
}

const TerrainMaterial* TerrainDocument::GetMaterial(const std::string& name) const
{
    auto itr = MaterialLibrary.find(name);
    if (itr == MaterialLibrary.end())
        return nullptr;

    return &itr->second;
}
