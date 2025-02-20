#pragma once

#include "Panel.h"

#include <list>
#include <string>

namespace EditorFramework
{
    struct LogItem
    {
        int Level = 0;
        std::string Prefix;
        std::string Text;
        ImVec4 Color = { 1,1,1,1 };
    };

    namespace LogSink
    {
        void Init();
        bool HasItems();
        LogItem PopItem();
    }

    class LogPanel : public Panel
    {
    private:
        std::list<LogItem> Items;

        bool ScrollToBottom = false;

        int ShowLevel = 0;
        char FilterText[128] = { 0 };

    public:
        LogPanel();
        void OnShow() override;
    };
}