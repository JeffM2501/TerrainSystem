#include "KeybindingsDialog.h"
#include "Action.h"
#include "DisplayScale.h"

#include "raylib.h"

#include "ImGuiExtras.h"
#include "extras/IconsFontAwesome6.h"
#include "imgui_internal.h"

namespace EditorFramework
{
    DialogResult KeybindingDialog::OnShow()
    {
        if (KeyMode)
        {
            {
                auto action = ActionRegistry::GetAction(ActiveKeyAction);

                ImGui::Text("Press A Shortcut for %s", action->Name.c_str());
                ImGui::TextUnformatted(ImGui::GetKeyChordName(ImGui::GetIO().KeyMods));

                if (ImGui::Button("Cancel"))
                    KeyMode = false;

                auto func = [this](int i)
                    {
                        if (ImGui::IsKeyDown(ImGuiKey(i)))
                        {
                            ImGuiKeyChord newChord = ImGui::GetIO().KeyMods | ImGuiKey(i);

                            PendingChanges[ActiveKeyAction] = newChord;
                            KeyMode = false;
                        }
                    };

                for (int i = ImGuiKey::ImGuiKey_LeftArrow; i <= ImGuiKey::ImGuiKey_Escape; i++)
                    func(i);

                if (KeyMode)
                {
                    for (int i = ImGuiKey::ImGuiKey_0; i <= ImGuiKey::ImGuiKey_KeypadEqual; i++)
                        func(i);
                }
            }
        }
        else
        {
            ImGui::TextUnformatted("Key Bindings");
            if (ImGui::BeginTable("keytable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Action");
                ImGui::TableSetupColumn("Description");
                ImGui::TableSetupColumn("Default");
                ImGui::TableSetupColumn("Shortcut");
                ImGui::TableHeadersRow();

                for (auto& [id, action] : ActionRegistry::Registry)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::LabelTextLeft(action.Name.c_str());

                    ImGui::TableNextColumn();
                    ImGui::LabelTextLeft(action.Description.c_str());

                    ImGui::TableNextColumn();
                    ImGui::BeginDisabled(true);
                    ImGui::LabelTextLeft(ImGui::GetKeyChordName(action.DefaultShortcut));
                    ImGui::EndDisabled();

                    ImGui::TableNextColumn();

                    ImGui::PushID(int(id));
                    if (ImGui::Button(ICON_FA_PENCIL))
                    {
                        // start key mode
                        KeyMode = true;
                        ActiveKeyAction = id;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                   
                    bool isEdit = false;

                    ImGuiKeyChord chord = action.Shortcut;
                    if (PendingChanges.find(id) != PendingChanges.end())
                    {
                        chord = PendingChanges[id];
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0,1,0,1 });
                    }
                    else if (!ActionRegistry::IsActionDefault(id))
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    }
                    ImGui::LabelTextLeft(ImGui::GetKeyChordName(chord));
           
                    ImGui::PopStyleColor();
                }

                ImGui::EndTable();
            }
        }

        return DialogResult::None;
    }

    void KeybindingDialog::OnAccept()
    {
        for (auto& [id, chord] : PendingChanges)
        {
            ActionRegistry::SetActionKeybind(id, chord);
        }
    }

    KeybindingDialog::KeybindingDialog()
    {
        DialogName = "Set action key bindings";

        AcceptName = DialogOK;
        CancelName = DialogCancel;

        Resizeable = true;
        MinimumSize = ImVec2{ ScaleToDPI(640.0f), ScaleToDPI(400.0f) };
    }
}