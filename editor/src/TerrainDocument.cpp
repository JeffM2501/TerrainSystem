#include "TerrainDocument.h"
#include "EditorThumbnailManager.h"
#include "Application.h"
#include "DisplayScale.h"
#include "Dialog.h"

#include "imgui_utils.h"
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
using namespace Editor;

class NewTerrainDocumentDialog : public Dialog
{
private:
	AssetTypes::TerrainAsset* Terrain = nullptr;

	int GridSize = 128;
	float TileSize = 128;
	float MinZ = -50;
	float MaxZ = 100;

protected:
	DialogResult OnShow() override
	{
		if (ImGui::BeginTable("grid", 2, ImGuiTableFlags_SizingStretchSame, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0.25f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None, 0.75f);

			ImGui::TableNextRow(); ImGui::TableNextColumn();
			ImGui::TextUnformatted("Quads per tile"); ImGui::TableNextColumn(); ImGui::InputInt("(count)###Quads", &GridSize, 1, 10);

			ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::TextUnformatted("Tile Size"); ImGui::TableNextColumn(); ImGui::InputFloat("(m)###Size", &TileSize, 1, 10);

			ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::TextUnformatted("Minimum Z"); ImGui::TableNextColumn(); ImGui::InputFloat("(m)###MinZ", &MinZ, 1, 10);

			ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::TextUnformatted("Maximum Z"); ImGui::TableNextColumn(); ImGui::InputFloat("(m)###MaxZ", &MaxZ, 1, 10);

			ImGui::EndTable();
		}
		
		return DialogResult::None;
	}
public:
	NewTerrainDocumentDialog(AssetTypes::TerrainAsset* terrain, DialogCallback onAccept = nullptr, DialogCallback onCancel = nullptr)
	{
		DialogName = "Setup Terrain";

		Terrain = terrain;
		AcceptCallback = onAccept;
		CancelCallback = onCancel;

		AcceptName = "Create";
		CancelName = "Defaults";

		MinimumSize = ScaleToDPI(450, 150);
	}

	void OnAccept() override
	{
		// apply the data to the terrain
		uint8_t size = uint8_t(GridSize);
		Terrain->GetInfo().SetGridSize(size);
		Terrain->GetInfo().SetTileSize(TileSize);
		Terrain->GetInfo().SetMaxZ(MaxZ);
		Terrain->GetInfo().SetTMinZ(MinZ);

		Dialog::OnAccept();
	}
};

void TerrainDocument::OnUpdate(int width, int height)
{
	ViewportDocument::OnUpdate(width, height);

	SetShaderValue(TerrainShader, SunVectorLoc, SunVector, SHADER_UNIFORM_VEC3);
}	

void TerrainDocument::OnAssetCreate()
{
	Application::GetInstance().ShowDialogBox<NewTerrainDocumentDialog>(RootAsset);
}

void TerrainDocument::OnAssetOpen()
{
	RootAsset->GetMaterials().ValueList.OnValueChanged.Add([this](const ValueChangedEvent& event)
		{
			HandleMaterialListChangedEvent(event);
		}, Token.GetToken());

	auto& materials = RootAsset->GetMaterials();

	for (int i = 0; i < materials.Size(); i++)
	{
		MaterialRefs.push_back(std::make_unique<AssetReferenceResolver<AssetTypes::TerrainMaterialAsset>>(EditManager, materials[i].ValuePtr));
		RebuildMaterialIndex(i);
		MaterialRefs.back()->ReferenceDataChanged.Add([this, i](auto&) {RebuildMaterialIndex(i); }, Token.GetToken());
	}
}

void TerrainDocument::OnAssetDirty()
{

}

void TerrainDocument::HandleMaterialListChangedEvent(const ValueChangedEvent& event)
{
	// figure out what slot changed and add/remove/rebuild it
	if (event.RecordType == ValueChangedEvent::ValueRecordType::TypeListCleared)
	{
		MaterialListCache.clear();
	}
    else if (event.RecordType == ValueChangedEvent::ValueRecordType::TypeListItemRemoved)
    {
		int index = event.Path.Elements.back().Index;
		MaterialListCache.erase(MaterialListCache.begin() + index);
    }
    else if (event.RecordType == ValueChangedEvent::ValueRecordType::TypeListItemAdded)
    {
        int index = event.Path.Elements.back().Index;
        MaterialRefs.push_back(std::make_unique<AssetReferenceResolver<AssetTypes::TerrainMaterialAsset>>(EditManager, RootAsset->GetMaterials()[index].ValuePtr));
		RebuildMaterialIndex(index);
		MaterialRefs.back()->ReferenceDataChanged.Add([this, index](auto&) {RebuildMaterialIndex(index); }, Token.GetToken());
    }
	else if (event.RecordType == ValueChangedEvent::ValueRecordType::PrimitiveChanged)
    {
        int index = event.Path.Elements.back().Index;
        RebuildMaterialIndex(index);
    }
}

void TerrainDocument::RebuildMaterialIndex(int index)
{
    if (index >= MaterialListCache.size())
        MaterialListCache.resize(index+1);

	if (MaterialRefs[index]->IsValid())
	{
		auto& matAsset = MaterialRefs[index]->AssetValue->GetMaterial();

		MaterialListCache[index].DiffuseColor = matAsset.GetDiffuseColor();

		if (!matAsset.GetDiffuseTexture().GetPath().empty())
			MaterialListCache[index].DiffuseMap = ThumbnailManager::GetThumbnail(matAsset.GetDiffuseTexture().GetPath());
		else
			MaterialListCache[index].DiffuseMap.id = -1;

        if (!matAsset.GetNormalMap().GetPath().empty())
            MaterialListCache[index].NormalMap = ThumbnailManager::GetThumbnail(matAsset.GetNormalMap().GetPath());
        else
            MaterialListCache[index].NormalMap.id = -1;
	}
	else
	{
		MaterialListCache[index].DiffuseColor = WHITE;
		MaterialListCache[index].DiffuseMap.id = -1;
		MaterialListCache[index].NormalMap.id = -1;
	}
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
	auto size = ImGuiUtils::GetButtonSize(ICON_FA_BOX);

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
	ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
	ImGui::SetNextWindowSize(ImVec2(-1, -1));
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

void TerrainDocument::SetupDocument()
{
	TerrainShader = LoadShader("resources/shaders/terrain.vs", "resources/shaders/terrain.fs");
	if (!IsShaderValid(TerrainShader))
		TerrainShader = LoadShader(nullptr, nullptr);

	SunVectorLoc = GetShaderLocation(TerrainShader, "sunVector");

	NearPlane = 0.1f;
	FarPlane = 5000.0f;

	float selectedColor[4] = { 1,1,0,1 };
	SetShaderValue(TerrainShader, GetShaderLocation(TerrainShader, "selectedColor"), selectedColor, SHADER_UNIFORM_VEC4);

	SelectedShaderFlagLoc = GetShaderLocation(TerrainShader, "selected");
	ShowSplatFlagLoc = GetShaderLocation(TerrainShader, "showSplat");

	Renderer.SetShader(TerrainShader);

	auto visGroup = MainToolbar.AddGroup("TerrainVis");
	auto splatCommand = visGroup->AddItem<StateMenuCommand>(0, ICON_FA_SPLOTCH, "Show Splatmap", [this](CommandContextSet*) {ShowSplat = !ShowSplat; }, [this](CommandContextSet*) {return ShowSplat; });

	auto& cameraGroup = MainToolbar.AddGroup("Cameras");

	auto viewMenu = DocumentMenuBar.AddSubItem("View", "", 20);
	auto showGroup = viewMenu->AddGroup("Show", ICON_FA_EYE);
	showGroup->AddItem(0, splatCommand);
}

void TerrainDocument::OnCreated()
{
	SetupDocument();
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