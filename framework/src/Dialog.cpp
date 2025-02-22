#include "Dialog.h"

#include "DisplayScale.h"

#include "imgui.h"
#include "rlImGui.h"

namespace EditorFramework
{
    ImVec2 GetButtonSize(std::string_view label)
    {
        ImVec2 result = ImGui::CalcTextSize(label.data());
        result.x += (ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().FramePadding.x);
        return result;
    }

    static ImVec2 GetButtonsSize(std::string_view l1, std::string_view l2, std::string_view l3)
    {
        ImVec2 results(0, ImGui::GetTextLineHeight());

        int count = 0;
        if (!l1.empty())
        {
            count++;
            results.x += GetButtonSize(l1).x;
        }

        if (!l2.empty())
        {
            count++;
            results.x += GetButtonSize(l2).x;
        }

        if (!l3.empty())
        {
            count++;
            results.x += GetButtonSize(l3).x;
        }
        
        if (count > 1)
            results.x += ImGui::GetStyle().ItemInnerSpacing.x * (count-1);

        return results;
    }

    static void RightJustify(float width)
    {
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - width);
    }

    static void BottomJustify(float height)
    {
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y - (height + ImGui::GetStyle().WindowPadding.y));
    }

    bool Dialog::Process()
    {
        if (!Created)
            Create();

        OnUpdate();

        float footerSize = GetButtonSize("X").y + (ImGui::GetStyle().FramePadding.y*2) + ImGui::GetStyle().WindowPadding.y;
        ImVec2 minSize = MinimumSize;
        minSize.y += footerSize;

        ImGui::SetNextWindowSizeConstraints(minSize, ImVec2{ 999999,99999 });

        bool open = true;

        bool* openPtr = &open;

        if (CancelName.empty())
            openPtr = nullptr;

        DialogResult results = DialogResult::None;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
        if (!Resizeable)
            flags |= ImGuiWindowFlags_NoResize;

        if (ImGui::BeginPopupModal(ImGuiName.c_str(), openPtr, flags))
        {
            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            contentSize.y -= footerSize;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            bool show = ImGui::BeginChild(ImGuiName.c_str(), contentSize);
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            ImGui::BeginDisabled(!ChildDialogs.empty());
            if (show)
                results = OnShow();

            ImGui::EndDisabled();
            ImGui::EndChild();

            if (results == DialogResult::None)
            {
                float buttonWidth = GetButtonsSize(AcceptName, DeclineName, CancelName).x;

                RightJustify(buttonWidth);

                if (!AcceptName.empty())
                {
                    
                    bool enabled = EnableAccept();

                    ImGui::BeginDisabled(!enabled);

                    if (ImGui::Button(AcceptName.c_str()))
                    {
                        results = DialogResult::Accept;
                    }

                    if (enabled && (ImGui::IsKeyChordPressed(ImGuiKey_Enter) || ImGui::IsKeyChordPressed(ImGuiKey_KeypadEnter)))
                    {
                        results = DialogResult::Accept;
                    }

                    if (false && enabled)
                    {
                        auto *drawList = ImGui::GetWindowDrawList();

                        drawList->AddRect(ImGui::GetItemRectMin(),
                            ImGui::GetItemRectMax(),
                            ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive]),
                            ImGui::GetStyle().WindowRounding,
                            ImDrawFlags_None,
                            ScaleToDPI(1.0f));
                    }

                    ImGui::EndDisabled();
                    ImGui::SameLine();
                }

                if (!DeclineName.empty())
                {
                    
                    if (ImGui::Button(DeclineName.c_str()))
                    {
                        results = DialogResult::Decline;
                    }
                    ImGui::SameLine();
                }

                if (!CancelName.empty())
                {
                    if (ImGui::Button(CancelName.c_str()))
                    {
                        results = DialogResult::Cancel;
                    }
                }

                if (!open)
                    results = DialogResult::Cancel;
            }
			if (!ChildDialogs.empty())
            {
                if (ChildDialogs.front()->Process())
                    ChildDialogs.pop_front();
            }
            else
            {

            if (results != DialogResult::None)
            {
                OnResults(results);

                switch (results)
                {
                case DialogResult::Accept:
                    OnAccept();
                    break;

                case DialogResult::Decline:
                    OnDecline();
                    break;

                case DialogResult::Cancel:
                    OnCancel();
                    break;
                }

                ImGui::CloseCurrentPopup();
            }
        }

            ImGui::EndPopup();
        }

        return results != DialogResult::None;
    }

    EditorFramework::DialogResult Dialog::OnShow()
    {
        return DialogResult::None;
    }

    void Dialog::Create()
    {
        Created = true;
        OnCreate();

        ImGuiName = TextFormat("%s###%s_dialog_%p", DialogName.c_str(), DialogName.c_str(), this);

        ImGui::OpenPopup(ImGuiName.c_str(), ImGuiPopupFlags_None);
    }

    void Dialog::OnResults(DialogResult results)
    {
        if (ResultsCallback)
            ResultsCallback(*this, results);
    }

    void Dialog::OnAccept()
    {
        if (AcceptCallback)
            AcceptCallback(*this);
    }

    void Dialog::OnDecline()
    {
        if (DeclineCallback)
            DeclineCallback(*this);
    }

    void Dialog::OnCancel()
    {
        if (CancelCallback)
            CancelCallback(*this);
    }

    DialogResult SaveDirtyFileDialog::OnShow()
    {
        rlImGuiPushIconFont();
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_NavHighlight], ICON_FA_FLOPPY_DISK);
        ImGui::PopFont();

        ImGui::SameLine();
        ImGui::Text("The document \"%s\" has unsaved changes.\nDo you wish to save it?", Filename.c_str());

        return DialogResult::None;
    }

    SaveDirtyFileDialog::SaveDirtyFileDialog::SaveDirtyFileDialog(std::string_view file, DialogCallback onAccept, DialogCallback onDecline, DialogCallback onCancel)
        : Filename(file)
    {
        AcceptCallback = onAccept;
        DeclineCallback = onDecline;
        CancelCallback = onCancel;

        AcceptName = DialogYes;
        DeclineName = DialogNo;
        CancelName = DialogCancel;

        DialogName = "Save File?";

        MinimumSize = { ScaleToDPI(300.0f), ScaleToDPI(100.0f) };
    }
}