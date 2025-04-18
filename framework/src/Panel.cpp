#include "Panel.h"
#include "imgui.h"

namespace EditorFramework
{
    void Panel::Update()
    {
        if (!IsOpen())
            return;

        auto name = GetImGuiName();

        ImGui::SetNextWindowSizeConstraints(ImVec2(MiniumSize.x, MiniumSize.y), ImVec2(999999,999999));

        if (ForcedDockingID != 0)
        {
            if (ForcedDockingID == ImGuiID(-1))
                ImGui::SetNextWindowDockID(0);
            else
                ImGui::SetNextWindowDockID(ForcedDockingID);
        }

        if (ForceFocus || ForcedDockingID != 0)
        {   
            ImGui::SetNextWindowFocus();
        }

        ForceFocus = false;
        ForcedDockingID = 0;

        bool vis = ImGui::Begin(name.data(), &Shown);

        if (vis)
            OnShow();

        ImGui::End();
    }

    std::string_view Panel::GetImGuiName()
    {
        if (ImGuiName.empty())
        {
            ImGuiName = Icon + " " + Name + "###Panel" + Name;
        }

        return ImGuiName;
    }
}