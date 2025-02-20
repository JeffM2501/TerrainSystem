#include "TerrainInfoPanel.h"

#include "TerrainDocument.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"

using namespace EditorFramework;

TerrainInfoPanel::TerrainInfoPanel()
{
    Icon = ICON_FA_MOUNTAIN;
    Name = "Terrain Info";

    Location = PlanelLayoutLocation::Right;
}

void TerrainInfoPanel::OnShow()
{
    TerrainDocument* doc = GetDoumentAs<TerrainDocument>();

    if (!doc)
        return;

    ImGui::LabelTextLeft("Grid Size");

    int size = doc->Info.TerrainGridSize;
    if (ImGui::InputInt("###GridSize", &size, 16, 16))
    {
        if (size >= 16 && size < 256)
        {
            doc->Info.TerrainGridSize = uint8_t(size);
        }
    }
}
