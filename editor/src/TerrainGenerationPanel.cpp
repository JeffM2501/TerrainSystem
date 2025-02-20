#include "TerrainGenerationPanel.h"

#include "TerrainDocument.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
#include "DisplayScale.h"

using namespace EditorFramework;

TerrainGenerationPanel::TerrainGenerationPanel()
{
    Icon = ICON_FA_MAP;
    Name = "Generation";

    Location = PlanelLayoutLocation::Right;
}

void TerrainGenerationPanel::OnShow()
{
    TerrainDocument* doc = GetDoumentAs<TerrainDocument>();

    if (!doc)
        return;

    ImGui::LabelTextLeft("Size"); ImGui::SameLine();
    ImGui::SetNextItemWidth(ScaleToDPI(100.0f));
    ImGui::InputInt("###GridX", &GridX, 1, 1); ImGui::SameLine();
    ImGui::SetNextItemWidth(ScaleToDPI(100.0f));
    ImGui::InputInt("###GridY", &GridY, 1, 1); 

    ImGui::LabelTextLeft("PerlinScale"); ImGui::SameLine();
    ImGui::SetNextItemWidth(ScaleToDPI(100.0f));
    ImGui::InputFloat("###PerlinScale", &PerlinScale, 0.125f, 0.5f);

    if (ImGui::Button(ICON_FA_ARROW_UP_FROM_BRACKET " Generate"))
    {

        for (int y = 0; y < GridY; y++)
        {
            for (int x = 0; x < GridX; x++)
            {
                auto& tile = doc->Tiles.emplace_back(doc->Info);
                Image heightmap = GenImagePerlinNoise(131, 131, (x * 128) - 1, (y * 128) - 1, PerlinScale);
                tile.SetHeightsFromImage(heightmap);
                UnloadImage(heightmap);

                Image testSplat = GenImageChecked(65, 65, 2, 2, Color{ 255,0,0,0 }, Color{ 0,255,0,0 });
                ImageDrawRectangle(&testSplat, 16, 16, 32, 32, Color{ 0,0,0,0 });

                //ImageDrawCircle(&testSplat, 32, 32, 8, Color{0,0,0,0});

               // tile.Splatmap = LoadTextureFromImage(testSplat);
                UnloadImage(testSplat);

//                 tile.LayerMaterials.push_back(&GrassMateral);
//                 tile.LayerMaterials.push_back(&GroundMateral);
//                 tile.LayerMaterials.push_back(&RoadMateral);

                tile.Origin = TerrainPosition{ x, y };
               // builder.Build(tile);
            }
        }
    }

}
