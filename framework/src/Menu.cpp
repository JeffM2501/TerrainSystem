#include "Menu.h"
#include "Action.h"
#include "DisplayScale.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace EditorFramework
{
	void MenuBar::Show(bool isMain)
	{	
		bool show = false;
		if (isMain)
			show = ImGui::BeginMainMenuBar();
		else
			show = ImGui::BeginMenuBar();

		BarHeight = ImGui::GetWindowHeight();

		if (show)
		{
			ShowContents(this);

			if (isMain)
				ImGui::EndMainMenuBar();
			else
				ImGui::EndMenuBar();
		}
	}

	bool MenuBar::ProcessShortcuts(CommandContainer* container)
	{
		if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup))
			return true;

		if (container == nullptr )
			container = this;

        for (auto& item : container->Contents)
        {
            if (item->IsContainer())
            {
                CommandContainer* subContainer = static_cast<CommandContainer*>(item.get());
				if (ProcessShortcuts(subContainer))
					break;
            }
            else
            {
				CommandItem* command = static_cast<CommandItem*>(item.get());

				if (!command->IsEnabled(CommandContext))
					continue;

				if (ActionRegistry::IsActionTriggered(command->GetActionHash()))
				{
					command->Execute(1, CommandContext);
					return true;
				}
            }
        }
		return false;
	}

	void MenuBar::ShowContents(CommandContainer* container)
	{
		for (auto& item : container->Contents)
		{
			if (item->IsContainer())
			{
				CommandContainer* subContainer = static_cast<CommandContainer*>(item.get());
				if (subContainer->IsSubItem())
					ShowSubMenu(subContainer);
				else
					ShowGroup(subContainer);
			}
			else
			{
				ShowItem(static_cast<CommandItem*>(item.get()));
			}
		}
	}

	void MenuBar::ShowSubMenu(CommandContainer* container)
	{
        ImGui::Dummy(ImGui::GetStyle().FramePadding);
        ImGui::SameLine(0, 0);
        std::string menuText;
        if (!container->GetIcon().empty())
        {
            menuText += container->GetIcon().data();
            menuText += " ";
        }
        menuText += container->GetName().data();

		if (ImGui::BeginMenu(menuText.c_str()))
		{
			ShowContents(container);
			ImGui::EndMenu();
		}
	}

	void MenuBar::ShowGroup(CommandContainer* container)
	{
		if (container->Contents.empty())
			return;

		std::string menuText;
		if (!container->GetIcon().empty())
		{
			menuText += container->GetIcon().data();
			menuText += " ";
		}

		if (container->GetName().empty() || container->GetName().data()[0] == '#')
		{
			ShowContents(container);
			return;
		}

		auto color = ImGui::GetStyle().Colors[ImGuiCol_Header];
		ImGui::PushID(this);

			ImGui::TextColored(color, container->GetName().data());
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetTextLineHeight() / 2));
			ImGui::Separator();
		ImGui::PopID();
		ShowContents(container);
	}

	void MenuBar::ShowItem(CommandItem* item)
	{
		ImGui::Dummy(ImGui::GetStyle().FramePadding);
		ImGui::SameLine(0,0);

		std::string menuText;
		if (!item->GetIcon().empty())
		{
			menuText += item->GetIcon();
			menuText += " ";
		}
		menuText += item->GetName();

		bool selected = false;
		if (item->GetItemType() == CommandItem::ItemType::Toggle)
			selected = item->IsChecked();

		const char* shortcutName = nullptr;
		if (item->GetShortcut() != ImGuiKey_None)
			shortcutName = ImGui::GetKeyChordName(item->GetShortcut());

		switch (item->GetItemType())
		{
			case CommandItem::ItemType::Toggle:
			selected = item->IsChecked(CommandContext);
			[[fallthrough]];
			case CommandItem::ItemType::Button:
                if (ImGui::MenuItem(menuText.c_str(), shortcutName, selected, item->IsEnabled(CommandContext)))
                {
					item->Execute(1, CommandContext);
                }
			break;

			case CommandItem::ItemType::ValueInt:
			{
				ImGui::BeginDisabled(!item->IsEnabled(CommandContext));
				ImGui::TextUnformatted(menuText.c_str());
				ImGui::SameLine();
				menuText = "###" + menuText;
				int valueI = int(item->GetValue());
				if (ImGui::InputInt(menuText.c_str(), &valueI))
					item->Execute(float(valueI), CommandContext);
				ImGui::EndDisabled();
			}
            break;

			case CommandItem::ItemType::ValueFloat:
			{
				ImGui::BeginDisabled(!item->IsEnabled(CommandContext));
				ImGui::TextUnformatted(menuText.c_str());
				ImGui::SameLine();
				menuText = "###" + menuText;
				float valueF = item->GetValue();
				if (ImGui::InputFloat(menuText.c_str(), &valueF))
					item->Execute(valueF, CommandContext);
				ImGui::EndDisabled();
			}
            break;
		}
	}
}