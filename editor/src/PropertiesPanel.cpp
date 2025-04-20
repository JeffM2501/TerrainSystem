#include "PropertiesPanel.h"
#include "DisplayScale.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_internal.h"
#include "rlImGui.h"

#include <algorithm>

using namespace EditorFramework;
using namespace AttributeTypes;
using namespace Properties;
using namespace Types;

PropertiesPanel::PropertiesPanel()
{
	Icon = ICON_FA_LIST_UL;
	Name = "Properties";

	Location = PlanelLayoutLocation::Right;
}

static constexpr float IndentOffset = 5.0f;

void PropertiesPanel::ShowType(int indentLevel, TypeEditorCache& cache, Types::TypeValue* value)
{
	ImGui::PushID(cache.TypeDisplayName.c_str());

	ImGui::BeginDisabled(value->GetType()->HasAttribute<ReadOnlyAttribute>());

	for (const auto& [index, editor] : cache.FieldEditors)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::Dummy(ImVec2(indentLevel * ScaleToDPI(IndentOffset), 1));
		ImGui::SameLine(0, 0);

		if (editor.IsList)
		{
			if (ImGui::Button(ICON_FA_PLUS))
			{
				ListFieldValue& listValue = value->GetPrimitiveListFieldValue(index);
				listValue.Add();
			}
			ImGui::SetItemTooltip("Add item to %s list", editor.DisplayName.c_str());
			ImGui::SameLine();

            if ((ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
            {
                ImGui::SameLine();
                ImGui::BeginDisabled(value->GetListFieldCount(index) == 0);
                if (ImGui::Button(ICON_FA_TRASH))
                {
                    ListFieldValue& listValue = value->GetPrimitiveListFieldValue(index);
                    listValue.Clear();
                }
                ImGui::EndDisabled();
                ImGui::SetItemTooltip("Clear all items from %s list", editor.DisplayName.c_str());
            }

			ImGui::SameLine();
		}

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(editor.DisplayName.c_str());

		ImGui::TableNextColumn();
		ImGui::BeginDisabled(value->GetType()->FieldHasAttribute<ReadOnlyAttribute>(index));
		ImGui::PushID(editor.DisplayName.c_str());
		if (editor.IsList)
		{
			// primitive list

			for (int i = 0; i < value->GetListFieldCount(index); i++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::PushID(TextFormat("###%d", i));
				const char* label = TextFormat("%d", i + 1);
				if (ImGui::Button(ICON_FA_MINUS))
				{
					ListFieldValue& listValue = value->GetPrimitiveListFieldValue(index);
					listValue.Delete(i);
				}
				ImGui::SetItemTooltip("Remove index %d from %s list", i + 1, editor.DisplayName.c_str());

				ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x, 0);
				ImGui::TextUnformatted(label);
				ImGui::PopID();

				ImGui::TableNextColumn();
				ImGui::PushID(TextFormat("###%d", i));
				editor.Editor(value, index, i);
				ImGui::PopID();
			}
		}
		else
		{
			editor.Editor(value, index, -1);

		}
		ImGui::PopID();
		ImGui::EndDisabled();
	}

	for (auto& [index, editor] : cache.TypeEditors)
	{
		if (editor.CustomEditor)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::Dummy(ImVec2(indentLevel * ScaleToDPI(IndentOffset), 1));
			ImGui::SameLine(0, 0);

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(editor.DisplayName.c_str());

			ImGui::TableNextColumn();
			ImGui::BeginDisabled(value->GetType()->FieldHasAttribute<ReadOnlyAttribute>(index));
			ImGui::PushID(editor.DisplayName.c_str());
			editor.CustomEditor(value, index, -1);
			ImGui::PopID();
			ImGui::EndDisabled();
			continue;
		}
		auto& editorList = editor.Editors;

		auto& listValue = value->GetTypeListFieldValue(index);

		bool isList = value->GetType()->GetField(index)->IsList();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID(editor.DisplayName.c_str());

		if (isList)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered]);
			if (ImGui::Button(ICON_FA_PLUS))
			{
				listValue.Add();
				Registry.BuildCacheForListField(value, &cache, index);
			}
			ImGui::SetItemTooltip("Add item %s list", editor.DisplayName.c_str());

            ImGui::SameLine();
            ImGui::BeginDisabled(value->GetListFieldCount(index) == 0);
            if (ImGui::Button(ICON_FA_TRASH))
            {
                listValue.Clear();
            }
            ImGui::EndDisabled();
            ImGui::SetItemTooltip("Clear all items from %s list", editor.DisplayName.c_str());

			ImGui::PopStyleColor();
		}
		ImGui::PopID();

		ImGui::SameLine(0, 0);

		ImGui::Dummy(ImVec2(indentLevel * ScaleToDPI(IndentOffset), 1));
		ImGui::SameLine(0, 0);

		if (ImGui::CollapsingHeader(editor.DisplayName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_LabelSpanAllColumns))
		{
			// type list
			if (isList)
			{
				ImGui::PushID(editor.DisplayName.c_str());

				int indexToDelete = -1;
                for (int i = 0; i < value->GetListFieldCount(index); i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(TextFormat("###%d", i));
                    const char* label = TextFormat("%d", i + 1);
					if (ImGui::Button(ICON_FA_MINUS))
						indexToDelete = i;
                    
                    ImGui::SetItemTooltip("Remove index %d from %s list", i + 1, editor.DisplayName.c_str());

                    ImGui::SameLine();
					bool show = (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_LabelSpanAllColumns));
					ImGui::PopID();

					if (show)
					{
						ImGui::PushID(TextFormat("###%d", i));
						ShowType(indentLevel + 1, editorList[i], &listValue[i]);
						ImGui::PopID();
					}
					else
					{
						ImGui::TableNextRow();
					}
                }

				if (indexToDelete >= 0)
					listValue.Delete(indexToDelete);

				ImGui::PopID();
			}
			else
			{
				ImGui::BeginDisabled(value->GetType()->FieldHasAttribute<ReadOnlyAttribute>(index));
				ShowType(indentLevel + 1, editorList[0], value->GetTypeFieldValue(index));
				ImGui::EndDisabled();
			}
		}
	}
	ImGui::EndDisabled();
	ImGui::PopID();
}

void PropertiesPanel::OnShow()
{
	if (ObjectPointer == nullptr)
	{
		ImGui::TextUnformatted("No data");
		return;
	}

	if (AssetEditManager::GetCurrent())
		AssetEditManager::GetCurrent()->BeginEvent("Property Edit");

	if (ImGui::BeginTable("###PropertiesTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("PropertyName", ImGuiTableColumnFlags_None, 0.25f);
		ImGui::TableSetupColumn("PropertyValue", ImGuiTableColumnFlags_None, 0.75f);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::CollapsingHeader(TypeCache.TypeDisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_LabelSpanAllColumns))
		{
			ShowType(0, TypeCache, ObjectPointer);
		}

		ImGui::EndTable();
	}

	if (AssetEditManager::GetCurrent())
		AssetEditManager::GetCurrent()->FinalizeEvent();
}

void PropertiesPanel::SetObject(Types::TypeValue* value)
{
	ObjectPointer = value;
	TypeCache.Clear();
	Registry.BuildCacheForType(value, &TypeCache);
}
