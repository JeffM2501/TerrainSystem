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
    ImGui::InputFloat("###PerlinScale", &PerlinScale, 0.0125f, 0.125f);

    if (ImGui::Button(ICON_FA_ARROW_UP_FROM_BRACKET " Generate"))
    {
        doc->SetDirty();
        for (int y = 0; y < GridY; y++)
        {
            for (int x = 0; x < GridX; x++)
            {
                auto& tile = doc->GetTile(x, y);

                tile.UnloadGeometry();
                tile.UnloadSplats();

                Image heightmap = GenImagePerlinNoise(131, 131, (x * 128) - 1, (y * 128) - 1, PerlinScale);
                tile.SetHeightsFromImage(heightmap);
                UnloadImage(heightmap);

                Image testSplat = GenImageColor(65, 65, Color{ 0,0,0,255 });
                ImageDrawRectangle(&testSplat, 16, 16, 32, 32, Color{ 255, 0, 0, 255 });
                ImageDrawCircle(&testSplat, 32, 32, 8, Color{ 0,255,0,255 });
                ImageDrawCircle(&testSplat, 16, 32, 8, Color{ 0,0,255,255 });

                for (int y = 0; y < testSplat.height; y++)
                {
                    for (int x = 0; x < testSplat.width; x++)
                    {
                        Color c = GetImageColor(testSplat, x, y);
                        if (y > 45)
                            c.a = 255;
                        else
                            c.a = 0;

                        ImageDrawPixel(&testSplat, x, y, c);
                    }
                }

                tile.Splatmap = LoadTextureFromImage(testSplat);
                UnloadImage(testSplat);

                 tile.AddMaterial(doc->GetMaterial("Grass"));
                 tile.AddMaterial(doc->GetMaterial("Ground"));
                 tile.AddMaterial(doc->GetMaterial("Road"));
                 tile.AddMaterial(doc->GetMaterial("Snow"));
                 tile.AddMaterial(doc->GetMaterial("Grid"));


                tile.Origin = TerrainPosition{ x, y };
                Builder.Build(tile);
            }
        }
    }

}
