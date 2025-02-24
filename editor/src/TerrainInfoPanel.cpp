#include "TerrainInfoPanel.h"

#include "TerrainDocument.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"

#include <algorithm>

using namespace EditorFramework;

TerrainInfoPanel::TerrainInfoPanel()
{
    Icon = ICON_FA_MOUNTAIN;
    Name = "Info";

    Location = PlanelLayoutLocation::Right;
}

void TerrainInfoPanel::OnShow()
{
    TerrainDocument* doc = GetDoumentAs<TerrainDocument>();

    if (!doc)
        return;

    ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable;

    static constexpr char propertyTableName[] = "PropertyTable";

    // info
    if (ImGui::CollapsingHeader("Size Info", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTable(propertyTableName, 2, tableFlags))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::LabelTextLeft("Grid Size");
            ImGui::TableNextColumn();
            int size = doc->Info.TerrainGridSize;
            if (ImGui::InputInt("###GridSize", &size, 16, 16))
            {
                if (size >= 16 && size < 256)
                {
                    doc->Info.TerrainGridSize = uint8_t(size);
                    doc->SetDirty();
                }
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::LabelTextLeft("Min Z");
            ImGui::TableNextColumn();
            if (ImGui::InputFloat("###MinZ", &doc->Info.TerrainMinZ, 0.25f, 1))
                doc->SetDirty();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::LabelTextLeft("Max Z");
            ImGui::TableNextColumn();
            if (ImGui::InputFloat("###MaxZ", &doc->Info.TerrainMaxZ, 0.25f, 1))
                doc->SetDirty();

            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Tiles", ImGuiTreeNodeFlags_DefaultOpen))
    {
        size_t count = std::min(std::max(size_t(1),doc->Tiles.size()), size_t(10));

        float height = (ImGui::GetTextLineHeight() * count) + (count - 1) * ImGui::GetStyle().ItemInnerSpacing.y;

        if (ImGui::BeginChild("TileList", ImVec2(ImGui::GetContentRegionAvail().x, height)))
        {
            if (ImGui::BeginTable(propertyTableName, 2, tableFlags))
            {
                for (const auto& tile : doc->Tiles)
                {
                    const char* label = TextFormat("%d, %d", tile.Origin.X, tile.Origin.Y);

                    bool selected = doc->SelectedTileLoc == tile.Origin;
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    if (ImGui::Selectable(label, &selected, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        doc->SelectedTileLoc = tile.Origin;
                    }
           
                    ImGui::TableNextColumn();
                    ImGui::Text("%d layers", tile.LayerMaterials.size());
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }

}
