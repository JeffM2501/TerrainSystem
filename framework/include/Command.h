#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "Action.h"

namespace EditorFramework
{

    class CommandContainerItem
    {
    public:
        virtual ~CommandContainerItem() = default;

         inline virtual bool IsContainer() const { return false; }

         size_t SortIndex = 0;
    };

	class CommandItem : public CommandContainerItem
	{
    public:
        CommandItem(size_t actionHash) : ActionHash(actionHash) {};

        enum class ItemType
        {
            Button,
            Toggle,
            ValueInt,
            ValueFloat
        };

        virtual ItemType GetItemType() const { return ItemType::Button; }

        virtual bool IsShown() const { return true; }

        virtual bool IsEnabled() const { return true; }

        virtual bool IsChecked() const { return false; }

        virtual void Execute(float value = 0 ) = 0;
        virtual float GetValue() { return 0; }

        size_t GetActionHash() const { return ActionHash; }

        virtual std::string_view GetName() = 0;
		virtual std::string_view GetIcon() = 0;
        virtual std::string_view GetDescription() { return ""; }
		virtual ImGuiKeyChord GetShortcut() { return ImGuiKey_None; };

    protected:
        size_t ActionHash = 0;
	};

    class ActionCommandItem : public CommandItem
    {
    public:
        ActionCommandItem(size_t actionHash
            , std::function<void(float)> executeFunc
            , std::function<bool()> enableFunc = nullptr
            , std::function<bool()> checkedFunc = nullptr)
            : CommandItem(actionHash)
            , OnExecute(executeFunc)
            , OnEnable(enableFunc)
            , OnCheck(checkedFunc)
        {
            _ASSERT(ActionRegistry::GetAction(ActionHash));
        }

        ActionCommandItem(std::string_view actionName
            , std::function<void(float)> executeFunc
            , std::function<bool()> enableFunc = nullptr
            , std::function<bool()> checkedFunc = nullptr)
            : CommandItem(ActionRegistry::GetActionHash(actionName))
            , OnExecute(executeFunc)
            , OnEnable(enableFunc)
            , OnCheck(checkedFunc)
        {
            _ASSERT(ActionRegistry::GetAction(ActionHash));
        }

        std::function<void(float)> OnExecute;
        std::function<bool()> OnEnable;
        std::function<bool()> OnCheck;

        void Execute(float value = 0) override
        {
            if (OnExecute)
                OnExecute(value);
        }

        bool IsEnabled() const override
        {
            if (OnEnable)
                return OnEnable();

            return true;
        }

        bool IsChecked() const override 
        {
            if (OnCheck)
                return OnCheck();

            return false;
        }
        
        std::string_view GetName() override { return ActionRegistry::GetAction(ActionHash)->Name; }
        std::string_view GetIcon() override { return ActionRegistry::GetAction(ActionHash)->Icon; }
        std::string_view GetDescription() override { return ActionRegistry::GetAction(ActionHash)->Description; }
        ImGuiKeyChord GetShortcut()override { return ActionRegistry::GetAction(ActionHash)->Shortcut; }
    };

    class CommandContainer : public CommandContainerItem
    {
    public:
        CommandContainer(std::string_view name = "", std::string_view icon = "") : Name(name) ,Icon(icon) {}

        inline bool IsContainer() const override { return true; }
		inline virtual bool IsSubItem() const { return SubItem; }

        std::vector<std::shared_ptr<CommandContainerItem>> Contents;

        template<class T, class... Args>
        std::shared_ptr<T> AddItem(size_t insertLocation, Args&&... args)
        {
            auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
            ptr->SortIndex = insertLocation;

            auto itr = Contents.begin();

            while (itr != Contents.end())
            {
                if ((*itr)->SortIndex > insertLocation)
                {
                    Contents.insert(itr, ptr);
                    return ptr;
                }
                itr++;
            }
            Contents.push_back(ptr);
            return ptr;
        }

        std::shared_ptr<CommandItem> AddItem(size_t insertLocation, std::shared_ptr<CommandItem> ptr)
        {
            auto itr = Contents.begin();

            while (itr != Contents.end())
            {
                if ((*itr)->SortIndex > insertLocation)
                {
                    Contents.insert(itr, ptr);
                    return ptr;
                }
                itr++;
            }
            ptr->SortIndex = insertLocation;
            Contents.push_back(ptr);
            return ptr;
        }

        CommandContainer& AddGroup(std::string_view name, std::string_view icon = "", size_t insertLocation = 0)
		{
            for (auto& item : Contents)
            {
                if (item->IsContainer())
                {
                    CommandContainer* container = static_cast<CommandContainer*>(item.get());

                    if (container->Name == name)
                        return *container;
                }
            }

            CommandContainer& container = *AddItem<CommandContainer>(insertLocation, name, icon).get();
            return container;
		}

		CommandContainer& AddSubItem(std::string_view name, std::string_view icon = "", size_t insertLocation = 0)
		{
            CommandContainer& container = AddGroup(name, icon, insertLocation);
            container.SubItem = true;
            return container;
		}

        std::shared_ptr<CommandItem> FindItemByAction(size_t actionHash)
        {
            for (std::shared_ptr<CommandContainerItem> item : Contents)
            {
                if (item->IsContainer())
                {
                    std::shared_ptr<CommandItem> result = static_cast<CommandContainer*>(item.get())->FindItemByAction(actionHash);
                    if (result)
                    {
                        return result;
                    }
                }
                else
                {
                    CommandItem* itemPtr = static_cast<CommandItem*>(item.get());
                    if (itemPtr->GetActionHash() == actionHash)
                    {
                        return std::dynamic_pointer_cast<CommandItem>(item);
                    }
                }
            }

            return nullptr;
        }

        std::shared_ptr<CommandItem> FindItemByAction(std::string_view actionName)
        {
            return FindItemByAction(ActionRegistry::GetActionHash(actionName));
        }

        virtual std::string_view GetName() const { return Name; }
        virtual std::string_view GetIcon() const { return Icon; }

    protected:
        std::string Name;
        std::string Icon;

    private:
        bool SubItem = false;
    
    };
}