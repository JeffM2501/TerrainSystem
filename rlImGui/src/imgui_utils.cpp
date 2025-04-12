#include "imgui_utils.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGuiUtils
{
    constexpr ImVec4 Red{ 1,0,0,1 };
    constexpr ImVec4 Green{ 0,1,0,1 };
    constexpr ImVec4 Blue{ 0,0,1,1 };
    constexpr ImVec4 Yellow{ 1,0,1,1 };
    constexpr ImVec4 DarkRed{ 0.5f,0,0,1 };
    constexpr ImVec4 DarkGreen{ 0,0.5f,0,1 };
    constexpr ImVec4 DarkBlue{ 0,0,0.5f,1 };
    constexpr ImVec4 LightRed{ 1,0.5f,0.5f,1 };
    constexpr ImVec4 LightGreen{ 0.5f,1,0.5f,1 };
    constexpr ImVec4 LightBlue{ 0.5f,0.5f,1,1 };

    bool VectorFloatEditor(const char* label, float& value, ImVec4 color, float width)
    {
        const char* tag = TextFormat("###%s", label);

        bool modified = false;

        ImU32 colorI = ImGui::GetColorU32(ImVec4(color.x * 0.5f, color.y * 0.5f, color.z * 0.5f, color.w));
        ImU32 white = ImGui::GetColorU32(ImVec4(1,1,1,1));

        float frameHeight = ImGui::GetFrameHeight();

      //  if (ImGui::BeginChild(tag, ImVec2(width, frameHeight)))
        {
            auto* drawList = ImGui::GetForegroundDrawList();
            auto dummyStart = ImGui::GetCursorScreenPos();

            ImGui::Dummy(ImVec2(frameHeight * 0.75f, frameHeight));
            ImGui::SameLine(0, 0);

            auto start = ImGui::GetCursorScreenPos();
            dummyStart.y += 3;
            drawList->AddText(dummyStart, white, label);

            modified = ImGui::DragFloat(tag, &value, 1, 0, 0, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::SameLine(0, 0);

            drawList->AddLine(ImVec2{ start.x, start.y + ImGui::GetFrameHeight() }, ImVec2{ ImGui::GetCursorScreenPos().x,start.y + ImGui::GetFrameHeight() }, colorI, 2);
          
        }
     //   ImGui::EndChild();
        return modified;
    }

    bool Vector2Editor(Vector2& data)
    {
        ImGui::PushID(&data);
        float width = floorf(ImGui::GetContentRegionAvail().x / 2);

        bool modified = false;
        modified = VectorFloatEditor("X", data.x, Red, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Y", data.y, Green, width) || modified;

        ImGui::PopID();
        return modified;
    }

    bool Vector3Editor(Vector3& data)
    {
        ImGui::PushID(&data);
        float width = floorf(ImGui::GetContentRegionAvail().x/3);

        bool modified = false;
        modified = VectorFloatEditor("X", data.x, Red, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Y", data.y, Green, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Z", data.z, Blue, width) || modified;
        ImGui::PopID();
        return modified;
    }

    bool Vector4Editor(Vector4& data)
    {
        ImGui::PushID(&data);
        float width = floorf(ImGui::GetContentRegionAvail().x / 3);

        bool modified = false;
        modified = VectorFloatEditor("X", data.x, Red, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Y", data.y, Green, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Z", data.z, Blue, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("W", data.w, Yellow, width) || modified;
        ImGui::PopID();
        return modified;
    }

    bool RectangleEditor(Rectangle& data)
    {
        ImGui::PushID(&data);
        float width = floorf(ImGui::GetContentRegionAvail().x / 4);

        bool modified = false;
        modified = VectorFloatEditor("X", data.x, Red, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("Y", data.y, Green, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("W", data.width, LightRed, width) || modified; ImGui::SameLine(0, 0);
        modified = VectorFloatEditor("H", data.height, LightBlue, width) || modified;
        ImGui::PopID();
        return modified;
    }

    bool IsSpace(char aCharacter)
    {
        // all space characters are values 32 or less (space is 32)
        // so we can convert them to a bitmask and use a single condition
        const int mask = (1 << (' ' - 1)) | (1 << ('\f' - 1)) | (1 << ('\n' - 1)) | (1 << ('\r' - 1)) | (1 << ('\t' - 1)) | (1 << ('\v' - 1));
        return (mask & (1 << ((aCharacter && aCharacter <= 32) * (aCharacter - 1)))) != 0;
    }

    //-------------------------------------------------------------------------------------------------
    // Todo: Add support for soft-hyphens when using word boundaries?
    //-------------------------------------------------------------------------------------------------
    void TextWithEllipsis(std::string_view string, float aMaxWidth, bool useWordBoundaries, float aSpacing)
    {
        char const* partStart = string.data();
        char const* partEnd = string.data();

        ImWchar elipsisChar = ImGui::GetFont()->EllipsisChar;
        char elipsisText[8];
        ImTextStrToUtf8(elipsisText, sizeof(elipsisText), &elipsisChar, (&elipsisChar) + 1);

        if (aSpacing < 0.0f) aSpacing = ImGui::GetStyle().ItemSpacing.x;

        float const ellipsisWidth = ImGui::CalcTextSize(elipsisText).x + aSpacing;
        float width = 0;
        bool addElipsis = false;

        while (*partStart != 0 && size_t(partStart - string.data()) < string.size())
        {
            // Add space to next segment
            while (IsSpace(*partEnd))
                partEnd++;

            if (useWordBoundaries)
            {
                // get next 'word' by looking for space after non-space
                while (*partEnd != 0 && size_t(partEnd - string.data()) < string.size() && !IsSpace(*partEnd))
                    ++partEnd;
            }
            else
            {
                if (*partEnd != 0)
                    ++partEnd;
            }

            ImVec2 const wordSize = ImGui::CalcTextSize(partStart, partEnd);

            // Clearly we have space for this word so just add it
            if (wordSize.x + width + ellipsisWidth < aMaxWidth)
            {
                width += wordSize.x;
                partStart = partEnd;
            }
            // If we're just at the end of the word and we just fit then we can commit here
            else if (*partEnd == 0 && wordSize.x + width < aMaxWidth)
            {
                width += wordSize.x;
                partStart = partEnd;
            }
            // we're done so add elipsis where the current segment starts
            else
            {
                addElipsis = true;
                break;
            }
        }

        ImGui::TextUnformatted(string.data(), partStart);

        if (addElipsis)
        {
            ImGui::SameLine(0.0f, aSpacing);
            ImGui::TextUnformatted(elipsisText);
        }
    }

    ImVec2 GetButtonSize(std::string_view label)
    {
        ImVec2 result = ImGui::CalcTextSize(label.data());
        result.x += (ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().FramePadding.x);
        return result;
    }

    ImVec2 GetButtonsSize(std::string_view l1, std::string_view l2, std::string_view l3)
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
            results.x += ImGui::GetStyle().ItemInnerSpacing.x * (count - 1);

        return results;
    }

    void RightJustify(float width)
    {
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - width);
    }

    void BottomJustify(float height)
    {
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y - (height + ImGui::GetStyle().WindowPadding.y));
    }
}