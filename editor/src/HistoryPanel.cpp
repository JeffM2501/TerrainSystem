#include "HistoryPanel.h"
#include "AssetEditManager.h"
#include "extras/IconsFontAwesome6.h"


using namespace EditorFramework;
using namespace AttributeTypes;
using namespace Properties;
using namespace Types;

HistoryPanel::HistoryPanel()
{
    Icon = ICON_FA_CLOCK_ROTATE_LEFT;
    Name = "Edit History";

    Location = PlanelLayoutLocation::Right;
}

void HistoryPanel::OnShow()
{
    auto* manager = AssetEditManager::GetCurrent();

    if (!manager)
        return;

    ImGui::BeginChild("HistoryPanel", ImVec2(0, 0));

    auto& editEvents = manager->GetEditEvents();
    for (auto itr = editEvents.rbegin(); itr != editEvents.rend(); itr++)
    {
        auto& event = *itr;
        ImGui::PushID(event.get());

        const char* name = nullptr;
        if (event->FieldName.empty())
            name = event->Name.c_str();
        else
            name = TextFormat("%s [%s]", event->Name.c_str(), event->FieldName.c_str());

        ImGui::Selectable(name, false, ImGuiSelectableFlags_SpanAllColumns);
        ImGui::PopID();
    }

    ImGui::EndChild();
}
