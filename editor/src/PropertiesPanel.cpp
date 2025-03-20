#include "PropertiesPanel.h"
#include "DisplayScale.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
#include "rlImGui.h"

#include <algorithm>

using namespace EditorFramework;

using namespace Properties;
using namespace Types;

PropertiesPanel::PropertiesPanel()
{
	Icon = ICON_FA_LIST_UL;
	Name = "Properties";

	Location = PlanelLayoutLocation::Right;
}

void ShowType(int indentLevel, TypeEditorCache& cache, Types::TypeValue* value)
{
	for (const auto& [index, editor] : cache.FieldEditors)
	{
		ImGui::TextUnformatted(editor.DisplayName.c_str());
		ImGui::SameLine();
		editor.Editor(value, index, true);
	}

	for (const auto& [index, editorList] : cache.TypeEditors)
	{
		if (value->GetType()->GetField(index)->GetType() == FieldType::TypeList)
		{

		}
		else
		{

		}
	}
}

void PropertiesPanel::OnShow()
{
	if (ObjectPointer == nullptr)
	{
		ImGui::TextUnformatted("No data");
		return;
	}

	if (ImGui::CollapsingHeader(TypeCache.TypeDisplayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ShowType(0, TypeCache);
	}
}

void PropertiesPanel::SetObject(Types::TypeValue* value)
{
	ObjectPointer = value;
	TypeCache.Clear();
	Registry.BuildCacheForType(value, &TypeCache);
}
