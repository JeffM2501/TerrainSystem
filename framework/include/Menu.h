#pragma once

#include "Command.h"

namespace EditorFramework
{
	class MenuBar : public CommandContainer
	{
	public:
		virtual bool IsSubItem() const override { return true; }

		void Show(bool isMain = false);

		bool ProcessShortcuts(CommandContainer* container = nullptr);

		float GetHeight() const { return BarHeight; }

	protected:

		void ShowContents(CommandContainer& container);

		void ShowSubMenu(CommandContainer& container);
		void ShowGroup(CommandContainer& container);

		void ShowItem(CommandItem& item);

		float BarHeight = 0;
	};

	class WindowStateMenuCommand : public CommandItem
	{
    private:
        std::string Icon;
        std::string WindowName;

    public:
        WindowStateMenuCommand(std::string_view icon, std::string_view name, std::function<void()> toggle, std::function<bool()> check)
            : CommandItem(0)
            , Icon(icon)
            , WindowName(name)
            , OnToggle(toggle)
            , OnChecked(check)
        {
        }

        ItemType GetItemType() const override{ return ItemType::Toggle; }

        std::function<void()> OnToggle;
        std::function<bool()> OnChecked;

        void Execute(float value = 0) override
        {
            if (OnToggle)
                OnToggle();
        }

        bool IsChecked() const override
        {
            if (OnChecked)
                return OnChecked();

            return false;
        }

        std::string_view GetName() override { return WindowName; }
        std::string_view GetIcon() override { return Icon; }
    };
}