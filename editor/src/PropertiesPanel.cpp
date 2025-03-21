#include "PropertiesPanel.h"
#include "DisplayScale.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
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

void ShowType(int indentLevel, TypeEditorCache& cache, Types::TypeValue* value)
{
	ImGui::PushID(cache.TypeDisplayName.c_str());

	ImGui::BeginDisabled(value->GetType()->HasAttribute<ReadOnlyAttribute>());

	for (const auto& [index, editor] : cache.FieldEditors)
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
		editor.Editor(value, index, true);
		ImGui::PopID();
		ImGui::EndDisabled();
	}

	for (auto& [index, editor] : cache.TypeEditors)
	{
		auto& editorList = editor.Editors;
    
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
		if (value->GetType()->GetField(index)->GetType() == FieldType::TypeList)
		{
			// TODO, handle lists
		}
		else
		{
            ImGui::Dummy(ImVec2(indentLevel * ScaleToDPI(IndentOffset), 1));
            ImGui::SameLine(0, 0);

			if (ImGui::CollapsingHeader(editor.DisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_LabelSpanAllColumns))
			{
				ShowType(indentLevel + 1, editorList[0], value->GetTypeFieldValue(index));
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
}

void PropertiesPanel::SetObject(Types::TypeValue* value)
{
	ObjectPointer = value;
	TypeCache.Clear();
	Registry.BuildCacheForType(value, &TypeCache);
}
