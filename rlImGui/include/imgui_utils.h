#pragma once

#include <string>
#include "raylib.h"

namespace ImGuiUtils
{
    void TextWithEllipsis(std::string_view string, float maxWidth, bool useWordBoundaries = false, float aSpacing = 0);
    
    bool RectangleEditor(Rectangle& data);
    bool Vector3Editor(Vector3& data);
    bool Vector2Editor(Vector2& data);
    bool Vector4Editor(Vector4& data);
}