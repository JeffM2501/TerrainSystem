#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "Action.h"
#include "CRC64.h"

namespace EditorFramework
{
#define DEFEINE_CONTEXT_ITEM(T)\
    static uint64_t ID() {return Hashes::CRC64Str(#T);}\
    uint64_t GetID() override  {return Hashes::CRC64Str(#T);}

    class CommandContextItem
    {
    public:
        virtual uint64_t GetID() = 0;
    };

    class CommandContextSet
    {
    public:
        std::unordered_map<uint64_t, std::unique_ptr<CommandContextItem>> Contetents;
        std::vector<CommandContextSet*> LinkedSets;

        void ClearLinkedSets() { LinkedSets.clear(); }
        void PushSet(CommandContextSet* set) { LinkedSets.push_back(set); }
        void PopSet() { LinkedSets.pop_back(); }

        template <class T, typename ...Args>
        T* AddItem(Args&&... args)
        {
            auto item = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = item.get();

            Contetents.insert_or_assign(T::ID(), std::move(item));
            return ptr;
        }

        template<class T>
        const T* GetItem() const
        {
            auto itr = Contetents.find(T::ID());
            if (itr != Contetents.end())
                return static_cast<T*>(itr->second.get());

            for (auto* link : LinkedSets)
            {
                T* ptr = link->GetItem<T>();
                if (ptr != nullptr)
                    return ptr;
            }
            return nullptr;
        }
    };

    class CommandContainerItem
    {
    public:
        virtual ~CommandContainerItem() = default;

         inline virtual bool IsContainer() const { return false; }

         size_t SortIndex = 0;
		 class CommandContainerItem* Parent = nullptr;

         class CommandContainerItem* GetRoot()
         {
             if (Parent)
                 return Parent->GetRoot();

             return this;
         }
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

        virtual bool IsShown(CommandContextSet *context = nullptr) const { return true; }

        virtual bool IsEnabled(CommandContextSet* context = nullptr) const { return true; }

        virtual bool IsChecked(CommandContextSet* context = nullptr) const { return false; }

        virtual void Execute(float value = 0, CommandContextSet* context = nullptr) = 0;
        virtual float GetValue(CommandContextSet* context = nullptr) { return 0; }

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
            , std::function<void(float, CommandContextSet*)> executeFunc
            , std::function<bool(CommandContextSet*)> enableFunc = nullptr
            , std::function<bool(CommandContextSet*)> checkedFunc = nullptr)
            : CommandItem(actionHash)
            , OnExecute(executeFunc)
            , OnEnable(enableFunc)
            , OnCheck(checkedFunc)
        {
            _ASSERT(ActionRegistry::GetAction(ActionHash));
        }

        ActionCommandItem(std::string_view actionName
            , std::function<void(float, CommandContextSet*)> executeFunc
            , std::function<bool(CommandContextSet*)> enableFunc = nullptr
            , std::function<bool(CommandContextSet*)> checkedFunc = nullptr)
            : CommandItem(ActionRegistry::GetActionHash(actionName))
            , OnExecute(executeFunc)
            , OnEnable(enableFunc)
            , OnCheck(checkedFunc)
        {
            _ASSERT(ActionRegistry::GetAction(ActionHash));
        }

        std::function<void(float, CommandContextSet*)> OnExecute;
        std::function<bool(CommandContextSet*)> OnEnable;
        std::function<bool(CommandContextSet*)> OnCheck;

        void Execute(float value = 0, CommandContextSet* context = nullptr) override
        {
            if (OnExecute)
                OnExecute(value, context);
        }

        bool IsEnabled(CommandContextSet* context = nullptr) const override
        {
            if (OnEnable)
                return OnEnable(context);

            return true;
        }

        bool IsChecked(CommandContextSet* context = nullptr) const override
        {
            if (OnCheck)
                return OnCheck(context);

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
            ptr->Parent = this;

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
            ptr->Parent = this;
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

        std::shared_ptr<CommandContainer> AddGroup(std::string_view name, std::string_view icon = "", size_t insertLocation = 0)
		{
            for (auto& item : Contents)
            {
                if (item->IsContainer())
                {
                    auto container = std::static_pointer_cast<CommandContainer>(item);

                    if (container->Name == name)
                        return container;
                }
            }

            auto container = AddItem<CommandContainer>(insertLocation, name, icon);
            return container;
		}

        std::shared_ptr<CommandContainer> AddSubItem(std::string_view name, std::string_view icon = "", size_t insertLocation = 0)
		{
            auto container = AddGroup(name, icon, insertLocation);
            container->SubItem = true;
            return container;
		}

        std::shared_ptr<CommandContainer> InsertContainer(size_t insertLocation, std::shared_ptr<CommandContainer> ptr)
        {
            auto itr = Contents.begin();
            ptr->Parent = this;
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