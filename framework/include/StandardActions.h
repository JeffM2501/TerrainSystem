#pragma once

#include "Action.h"
#include "extras/IconsFontAwesome6.h"

namespace EditorFramework
{
    static constexpr char ExitAction[] = "Exit";
    static constexpr char OpenAction[] = "Open...";
    static constexpr char CloseAction[] = "Close";
    static constexpr char CloseAllAction[] = "Close All";
    static constexpr char CloseAllButAction[] = "Close Others";
    static constexpr char SaveAction[] = "Save...";
    static constexpr char SaveAsAction[] = "Save As...";

    static constexpr char ImGuiDemoAction[] = "Demo Window";
    static constexpr char ImGuiStyleAction[] = "Style Editor";
    static constexpr char ImGuiMetricsAction[] = "Metrics Window";
    static constexpr char ImGuiItemPickerAction[] = "Item Picker";

    static constexpr char ResetLayoutAction[] = "Reset Layout";

    static constexpr char PreferencesAction[] = "Preferences...";
    static constexpr char ShortcutKeysAction[] = "Shortcut Keys...";

    inline void RegisterStandardActions()
    {
        ActionRegistry::Register(ExitAction, ICON_FA_DOOR_OPEN, "Quit the application", ImGuiKey_F4 | ImGuiMod_Alt);
        ActionRegistry::Register(OpenAction, ICON_FA_FILE_ARROW_UP, "Open Asset", ImGuiKey_O | ImGuiMod_Ctrl);

        ActionRegistry::Register(CloseAction, ICON_FA_DOOR_CLOSED, "Close Document", ImGuiKey_W | ImGuiMod_Ctrl);
        ActionRegistry::Register(CloseAllAction, ICON_FA_WINDOW_RESTORE, "Close All Documents", ImGuiKey_W | ImGuiMod_Ctrl | ImGuiMod_Alt);
        ActionRegistry::Register(CloseAllButAction, ICON_FA_RECTANGLE_XMARK, "Close All Other Documents", ImGuiKey_W | ImGuiMod_Ctrl | ImGuiMod_Shift);

        ActionRegistry::Register(SaveAction, ICON_FA_FLOPPY_DISK, "Save Asset...", ImGuiKey_S | ImGuiMod_Ctrl);
        ActionRegistry::Register(SaveAsAction, ICON_FA_SHARE_FROM_SQUARE, "Save Asset As...", ImGuiKey_S | ImGuiMod_Ctrl | ImGuiMod_Alt);

        ActionRegistry::Register(ImGuiDemoAction, ICON_FA_WINDOW_MAXIMIZE, "Show ImGui Demo Window", ImGuiKey_None);
        ActionRegistry::Register(ImGuiStyleAction, ICON_FA_PEN_FANCY, "Show ImGi Style Editor", ImGuiKey_None);
        ActionRegistry::Register(ImGuiMetricsAction, ICON_FA_RULER_COMBINED, "Show ImGui Metrics Window", ImGuiKey_None);
        ActionRegistry::Register(ImGuiItemPickerAction, ICON_FA_BUG_SLASH, "Break in debugger on next item click", ImGuiKey_None); 
        
        ActionRegistry::Register(ResetLayoutAction, ICON_FA_RECYCLE, "Reset Layout", ImGuiKey_None);

        ActionRegistry::Register(PreferencesAction, ICON_FA_LIST_CHECK, "System Preferences", ImGuiKey_None);
        ActionRegistry::Register(ShortcutKeysAction, ICON_FA_KEYBOARD, "Set Shortcut Keys", ImGuiKey_None);
    }
}
