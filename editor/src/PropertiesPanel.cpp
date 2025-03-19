#include "PropertiesPanel.h"
#include "DisplayScale.h"

#include "extras/IconsFontAwesome6.h"
#include "ImGuiExtras.h"
#include "rlImGui.h"

#include <algorithm>

using namespace EditorFramework;

PropertiesPanel::PropertiesPanel()
{
	Icon = ICON_FA_LIST_UL;
	Name = "Properties";

	Location = PlanelLayoutLocation::Right;
}

void PropertiesPanel::OnShow()
{
	if (ObjectPointer == nullptr)
	{
		ImGui::TextUnformatted("No data");
		return;
	}

	if (ImGui::CollapsingHeader(ObjectPointer->GetType()->TypeName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}

void PropertiesPanel::SetObject(Types::TypeValue* value)
{
	ObjectPointer = value;
}
