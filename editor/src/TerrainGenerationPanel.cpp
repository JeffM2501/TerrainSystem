#include "TerrainGenerationPanel.h"

#include "extras/IconsFontAwesome6.h"

using namespace EditorFramework;

TerrainGenerationPanel::TerrainGenerationPanel()
{
    Icon = ICON_FA_MAP;
    Name = "Terrain Generation";

    Location = PlanelLayoutLocation::Right;
}