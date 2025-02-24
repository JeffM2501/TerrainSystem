#include "DefaultPanels.h"

#include "extras/IconsFontAwesome6.h"

#include "rlImGuiColors.h"
#include "ImGuiExtras.h"

#include <deque>

namespace EditorFramework
{
    namespace LogSink
    {
        static std::deque<LogItem> LogItems;

        static char* stristr(const char* str1, const char* str2)
        {
            const char* p1 = str1;
            const char* p2 = str2;
            const char* r = *p2 == 0 ? str1 : 0;

            while (*p1 != 0 && *p2 != 0)
            {
                if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
                {
                    if (r == 0)
                    {
                        r = p1;
                    }

                    p2++;
                }
                else
                {
                    p2 = str2;
                    if (r != 0)
                    {
                        p1 = r + 1;
                    }

                    if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
                    {
                        r = p1;
                        p2++;
                    }
                    else
                    {
                        r = 0;
                    }
                }

                p1++;
            }

            return *p2 == 0 ? (char*)r : 0;
        }


        static const char* GetLogLevelName(int logLevel)
        {
            switch (logLevel)
            {
            default:            return "All";
            case LOG_TRACE:     return "Trace";
            case LOG_DEBUG:     return "DEBUG";
            case LOG_INFO:      return "Info";
            case LOG_WARNING:   return "Warning";
            case LOG_ERROR:     return "ERROR";
            case LOG_FATAL:     return "FATAL";
            }
        }

        static void GetLogLevelPrefix(int logLevel, LogItem& item)
        {
            item.Prefix = GetLogLevelName(logLevel);
            item.Prefix += ": ";
            switch (logLevel)
            {
            default:            item.Prefix.clear();
                item.Color = rlImGuiColors::Convert(WHITE); break;

            case LOG_TRACE:     item.Color = rlImGuiColors::Convert(GRAY); break;
            case LOG_DEBUG:     item.Color = rlImGuiColors::Convert(SKYBLUE); break;
            case LOG_INFO:      item.Color = rlImGuiColors::Convert(GREEN); break;
            case LOG_WARNING:   item.Color = rlImGuiColors::Convert(YELLOW); break;
            case LOG_ERROR:     item.Color = rlImGuiColors::Convert(ORANGE); break;
            case LOG_FATAL:     item.Color = rlImGuiColors::Convert(RED); break;
            }
        }

        static void TraceLogCB(int logLevel, const char* text, va_list args)
        {
            static char logText[2048] = { 0 };
            LogItem item;
            item.Level = logLevel;
            GetLogLevelPrefix(logLevel, item);
            vsprintf(logText, text, args);
            item.Text += logText;
            LogItems.emplace_back(std::move(item));
        }

        void Init()
        {
            SetTraceLogCallback(TraceLogCB);
        }

        bool HasItems()
        {
            return !LogItems.empty();
        }

        LogItem PopItem()
        {
            LogItem item = std::move(LogItems.front());
            LogItems.pop_front();
            return item;
        }
    }

    LogPanel::LogPanel()
    {
        Icon = ICON_FA_LIST;
        Name = "Log";

        Location = PlanelLayoutLocation::Bottom;

        Vector2 MiniumSize = { 300, 300 };
    }

    void LogPanel::OnShow()
    {
        ImGui::LabelTextLeft("Show:");
        ImGui::SameLine(0,0);

        ImGui::SetNextItemWidth(150);
        if (ImGui::BeginCombo("##LogLevel", LogSink::GetLogLevelName(ShowLevel)))
        {
            for (int i = 0; i < LOG_NONE; ++i)
            {
                bool is_selected = i == ShowLevel;
                if (ImGui::Selectable(LogSink::GetLogLevelName(i), is_selected))
                    ShowLevel = i;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("###filterText", "Filter", FilterText, 512);

        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            Items.clear();
        }

        ImVec2 size(ImGui::GetContentRegionAvail());
        size.y -= (ImGui::GetTextLineHeight()*2);

        if (ImGui::BeginChild("###LogChild", size))
        {
            while (LogSink::HasItems())
            {
                Items.push_back(LogSink::PopItem());
                ScrollToBottom = true;
            }

            while (Items.size() > 250)
                Items.pop_front();

            std::string copyBuffer;

            for (auto& line : Items)
            {
                if (ShowLevel != 0 && ShowLevel != line.Level)
                    continue;

                if (FilterText[0] != '\0')
                {
                    if (LogSink::stristr(line.Text.c_str(), FilterText) == nullptr)
                        continue;
                }

                ImGui::TextColored(line.Color, "%s", line.Prefix.c_str());
                ImGui::SameLine();
                ImGui::TextUnformatted(line.Text.c_str());
            }

            if (ScrollToBottom)
                ImGui::SetScrollHereY(1.0f);

            ScrollToBottom = false;
           
        }
        ImGui::EndChild();
    }
}