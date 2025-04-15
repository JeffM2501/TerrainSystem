#include "Toolbar.h"
#include "DisplayScale.h"

#include "raylib.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace EditorFramework
{
    void Toolbar::Show()
    {
        if (Contents.empty())
            return;

        const char* name = TextFormat("###Toolbar%p", this);

        BarHeight = ImGui::GetTextLineHeight() + (ImGui::GetStyle().FramePadding.y * 4);

        bool show = ImGui::BeginChild(name, ImVec2(ImGui::GetContentRegionAvail().x, BarHeight));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
        if (show)
        {
            ImGui::Dummy(ImGui::GetStyle().ItemInnerSpacing);
            ShowContents(*this);
        }
        ImGui::EndChild();
        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();
        auto* drawList = ImGui::GetWindowDrawList();

        drawList->AddLine(ImVec2(min.x, max.y), ImVec2(max.x, max.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Button]), ScaleToDPI(1.0f));
    }

    bool Toolbar::ProcessShortcuts(CommandContainer* container /*= nullptr*/)
    {
        return false;
    }

    void Toolbar::ShowContents(CommandContainer& container, int level)
    {
        for (size_t i = 0; i < container.Contents.size(); i++)
        {
            auto& item = container.Contents[i];
            if (item->IsContainer())
            {
                int levelToUse = level;
                if (level == 0 && i == container.Contents.size() - 1)
                    levelToUse++;

                ShowGroup(static_cast<CommandContainer&>(*item.get()), levelToUse);
            }
            else
            {
                ShowItem(static_cast<CommandItem&>(*item.get()));
            }
        }
    }

    void Toolbar::ShowGroup(CommandContainer& container, int level)
    {
        ImGui::PushID(container.GetName().data());
        ShowContents(container, level + 1);

        if (level == 0)
        {
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        }
        ImGui::PopID();
    }

    void Toolbar::ShowItem(CommandItem& item)
    {
        ImGui::SameLine(0, ImGui::GetStyle().CellPadding.x);

        const char* imGuiName = TextFormat("%s###%s", item.GetIcon().data(), item.GetName().data());

        const char* shortcutName = nullptr;
        if (item.GetShortcut() != ImGuiKey_None)
            shortcutName = ImGui::GetKeyChordName(item.GetShortcut());

        bool selected = false;

        ImGui::BeginDisabled(!item.IsEnabled(CommandContext));

        switch (item.GetItemType())
        {
        case CommandItem::ItemType::Toggle:
        {
            bool state = item.IsChecked(CommandContext);

            if (state)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

            if (ImGui::Button(imGuiName))
            {
                item.Execute(state ? 0.0f : 1.0f, CommandContext);
            }

            if (state)
            {
                auto color = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TabSelectedOverline]);
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, ImGui::GetStyle().FrameRounding, 0, ScaleToDPI(2.0f));
            }

            if (state)
                ImGui::PopStyleColor();

            break;
        }

        case CommandItem::ItemType::Button:
            if (ImGui::Button(imGuiName))
            {
                item.Execute(1);
            }
            break;

        case CommandItem::ItemType::ValueInt:
        {
            int valueI = int(item.GetValue());
            if (ImGui::InputInt(item.GetName().data(), &valueI))
                item.Execute(float(valueI), CommandContext);
        }
        break;

        case CommandItem::ItemType::ValueFloat:
        {
            float valueF = item.GetValue();
            if (ImGui::InputFloat(item.GetName().data(), &valueF))
                item.Execute(valueF, CommandContext);
        }
        break;
        }
        ImGui::EndDisabled();


        std::string toolTip(item.GetName());

        std::string_view description = item.GetDescription();
        if (!description.empty())
        {
            toolTip += "\n";
            toolTip += description;
        }

        if (shortcutName)
        {
            toolTip += "\n";
            toolTip += shortcutName;
        }

        ImGui::SetItemTooltip(toolTip.c_str());
    }
}