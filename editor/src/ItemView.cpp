#include "ItemView.h"

#include "imgui.h"
#include "imgui_utils.h"
#include "rlImGui.h"
#include "rlImGuiColors.h"
#include "raylib.h"

ViewableItem* ListItemView::Show(ViewableItemContainer& container)
{
	ViewableItem* item = container.Reset();

	ViewableItem* selected = nullptr;
	if (ImGui::BeginTable("ListTable", 3, ImGuiTableFlags_SizingFixedFit))
	{
		while (item)
		{
			if (item->Shown && !item->Filtered)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				bool isSelected = false;
				ImGui::Selectable(TextFormat("###%s", item->Name.c_str()), &isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
				if (isSelected && ImGui::IsMouseDoubleClicked(0))
				{
					selected = item;
				}
				ImGui::TableNextColumn();
				if (item->Tint.w > 0)
					ImGui::TextColored(item->Tint, " %s", item->Icon.c_str());
				else
					ImGui::Text(" %s", item->Icon.c_str());

				ImGui::TableNextColumn();
				ImGui::Text(" %s", item->Name.c_str());
			}

			item = container.Next();
		}
		ImGui::EndTable();
	}
	return selected;
}

bool GridItemView::ShowItem(ViewableItem* item)
{
	if (!item)
		return false;

	ImGui::PushID(item);
	bool selected = false;

	// icon tint
	if (item->Tint.w > 0)
		ImGui::PushStyleColor(ImGuiCol_Text, item->Tint);

	// big icon
	rlImGuiPushIconFont();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // no background on the button

	// center the button
	float size = ImGui::CalcTextSize(item->Icon.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float off = (DesiredItemWidith - size) * 0.5f;
	if (off > 0.0f)
	{
		ImGui::Dummy(ImVec2(off, 1));
		ImGui::SameLine(0, 0);
	}

	ImGui::Button(item->Icon.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();

	if (item->Tint.w > 0)
		ImGui::PopStyleColor();

	// check for double click
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
	{
		selected = item;
	}

	// tooltip
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(item->GetToolTip());

	// text is not centered to get max size
	ImGuiUtils::TextWithEllipsis(item->Name, ImGui::GetContentRegionAvail().x, false, 2);

	ImGui::PopID();

	return selected;
}

ViewableItem* GridItemView::Show(ViewableItemContainer& container)
{
	int currentColumn = 0;
	int columns = 1;

	float actualWidth = DesiredItemWidith + ImGui::GetStyle().CellPadding.x * 2;
	if (ImGui::GetContentRegionAvail().x > DesiredItemWidith)
		columns = std::max(1, int(floorf(ImGui::GetContentRegionAvail().x / actualWidth)));

	ViewableItem* item = container.Reset();

	ViewableItem* selected = nullptr;

	if (!ImGui::BeginTable("GridTable", columns, ImGuiTableFlags_SizingStretchSame))
		return nullptr;

	ImGui::TableNextRow();
	while (item)
	{
		if (!item->Shown || item->Filtered)
		{
			item = container.Next();
			continue;
		}

		ImGui::TableNextColumn();
		{
			if (ShowItem(item))
			{
				selected = item;
			}
		}

		item = container.Next();

		// see if we can move to the next row or not
		currentColumn++;
		if (currentColumn >= columns)
		{
			currentColumn = 0;
			ImGui::TableNextRow();
		}
	}
	ImGui::EndTable();

	return selected;
}
