#pragma once
#include "Dialog.h"

#include <unordered_map>

namespace EditorFramework
{
    class KeybindingDialog : public Dialog
    {
    private:
        std::unordered_map<size_t, ImGuiKeyChord> PendingChanges;

        bool KeyMode = false;
        size_t ActiveKeyAction = 0;

    protected:
        DialogResult OnShow() override;

        bool EnableAccept() override { return !PendingChanges.empty(); }

        void OnAccept() override;

    public:
        KeybindingDialog();
    };
}