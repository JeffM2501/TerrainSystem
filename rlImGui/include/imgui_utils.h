#pragma once

#include <string>
#include <string_view>
#include "raylib.h"

#include "imgui.h"

namespace ImGuiUtils
{
    void TextWithEllipsis(std::string_view string, float maxWidth, bool useWordBoundaries = false, float aSpacing = 0);
    
    bool RectangleEditor(Rectangle& data);
    bool Vector3Editor(Vector3& data);
    bool Vector2Editor(Vector2& data);
    bool Vector4Editor(Vector4& data);


    ImVec2 GetButtonSize(std::string_view label);
    ImVec2 GetButtonsSize(std::string_view l1, std::string_view l2, std::string_view l3);
    void RightJustify(float width);
    void BottomJustify(float height);
}