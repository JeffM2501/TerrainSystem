#pragma once

#include "imgui.h"

namespace ImGui
{
    inline void LabelTextLeft(const char* text, const char* end = nullptr)
    {
        auto labelSize = ImGui::CalcTextSize(text, end, true, -1);
        labelSize.x += ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::Dummy(labelSize);
        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();
        min.y += ImGui::GetStyle().ItemInnerSpacing.y / 2;

        auto color = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);

        ImGui::GetWindowDrawList()->AddText(min, color, text, end);
    }
}