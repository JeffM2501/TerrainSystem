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

		CommandContextSet* CommandContext = nullptr;

	protected:
		void ShowContents(CommandContainer* container);

		void ShowSubMenu(CommandContainer* container);
		void ShowGroup(CommandContainer* container);

		void ShowItem(CommandItem* item);

		float BarHeight = 0;
	};

	class StateMenuCommand : public CommandItem
	{
	private:
		std::string Icon;
		std::string WindowName;

	public:
		StateMenuCommand(std::string_view icon, std::string_view name, std::function<void(CommandContextSet*)> execute, std::function<bool(CommandContextSet*)> check)
			: CommandItem(0)
			, Icon(icon)
			, WindowName(name)
			, OnExecute(execute)
			, OnChecked(check)
		{
		}

		ItemType GetItemType() const override { return ItemType::Toggle; }

		std::function<void(CommandContextSet*)> OnExecute;
		std::function<bool(CommandContextSet*)> OnChecked;

		void Execute(float value = 0, CommandContextSet* context = nullptr) override
		{
			if (OnExecute)
				OnExecute(context);
		}

		bool IsChecked(CommandContextSet* context = nullptr) const override
		{
			if (OnChecked)
				return OnChecked(context);

			return false;
		}

		std::string_view GetName() const override { return WindowName; }
		std::string_view GetIcon() override { return Icon; }
	};
}