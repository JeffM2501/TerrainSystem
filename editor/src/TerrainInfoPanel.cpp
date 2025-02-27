#include "TerrainInfoPanel.h"

#include "TerrainDocument.h"

#include "DisplayScale.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
#include "rlImGui.h"

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
        size_t count = std::min(std::max(size_t(1),doc->Tiles.size()), size_t(5));

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

    if (ImGui::CollapsingHeader("Overview", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginChild("Map", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border))
        {
            ImGuiTableFlags gridFlags = ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_BordersInner;

            float gridSize = ScaleToDPI(48.0f);

            if (ImGui::BeginTable("TileGrid", int(doc->TerrainBounds.Y) + 1, gridFlags))
            {
                for (int h = 0; h <= doc->TerrainBounds.X; h++)
                {
                    ImGui::TableSetupColumn(TextFormat("%d", h), ImGuiTableColumnFlags_NoResize, gridSize);
                }

                if (!doc->Tiles.empty())
                {
                    for (int y = int(doc->TerrainBounds.Y); y >= 0; y--)
                    {
                        ImGui::TableNextRow();
                        for (int x = 0; x <= doc->TerrainBounds.X; x++)
                        {
                            ImGui::TableNextColumn();

                            auto pos = ImGui::GetCursorPos();
                            const char* label = TextFormat("%d, %d", x, y);
                            if (ImGui::Selectable(label, doc->SelectedTileLoc == TerrainPosition{ x,y }, 0, ImVec2(gridSize, gridSize)))
                            {
                                doc->SelectedTileLoc = TerrainPosition{ x,y };

                                auto& tile = doc->GetTile(x, y);

                                auto* focusController = doc->GetCamera().FindController<FocusCameraController>();
                                if (focusController)
                                {
                                    focusController->SetFocusPoint(doc->GetCamera(), 
                                        Vector3{float(x * tile.Info.TerrainGridSize),
                                                float(y * tile.Info.TerrainGridSize) ,
                                                float(tile.Info.TerrainMaxZ - tile.Info.TerrainMinZ + tile.Info.TerrainMinZ) },
                                        10.0f, 
                                        tile.Info.TerrainGridSize,
                                        false);
                                }
                            }

                            ImGui::SetCursorPos(pos);
                            ImGui::SetNextItemAllowOverlap();
                            if (doc->HasTile(x, y))
                            {
                                rlImGuiPushIconFont();
                                ImGui::TextUnformatted(ICON_FA_MOUNTAIN);
                                ImGui::PopFont();
                            }
                            else
                            {
                                ImGui::Dummy(ImVec2(gridSize, gridSize));
                            }
                            
                        }
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }

}
