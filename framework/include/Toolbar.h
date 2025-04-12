#pragma once

#include "Command.h"

namespace EditorFramework
{
    class Toolbar : public CommandContainer
    {
    public:
        void Show();

        bool ProcessShortcuts(CommandContainer* container = nullptr);

        float GetHeight() const { return BarHeight; }

        CommandContextSet* CommandContext = nullptr;

    protected:

        void ShowContents(CommandContainer& container, int leve = 0);
        void ShowGroup(CommandContainer& container, int level);

        void ShowItem(CommandItem& item);

        float BarHeight = 0;
    };
}