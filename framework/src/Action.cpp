#include "Action.h"

#include "imgui_internal.h"

namespace EditorFramework
{
	namespace ActionRegistry
	{
		std::unordered_map<size_t, Action> Registry;
		static std::hash<std::string_view> Hasher;

		size_t Register(std::string_view name, std::string_view icon, std::string_view description, ImGuiKeyChord shortcut)
		{
			size_t hash = Hasher(name);
			if (Registry.find(hash) != Registry.end())
				return hash;

			Action action;
			action.Name = name;
			action.Description = description;
			action.Icon = icon;
			action.Shortcut = shortcut;
			action.DefaultShortcut = shortcut;
			action.Hash = hash;
			Registry.insert_or_assign(hash, action);

			return hash;
		}

		size_t GetActionHash(std::string_view name)
		{
			return Hasher(name);
		}

		Action* GetAction(size_t hash)
		{
			auto itr = Registry.find(hash);
			if (itr == Registry.end())
				return nullptr;

			return &(itr->second);
		}

		Action* GetAction(std::string_view name)
		{
			return GetAction(Hasher(name));
		}

		bool IsActionTriggered(size_t hash)
		{
			auto* action = GetAction(hash);

			return action && action->Shortcut != ImGuiKey_None && ImGui::IsKeyChordPressed(action->Shortcut);
		}

		bool IsActionTriggered(std::string_view name)
		{
			return IsActionTriggered(Hasher(name));
		}

		bool IsActionHeld(size_t hash, bool anyMod)
		{
			auto* action = GetAction(hash);

			if (!action)
				return false;

			ImGuiKeyChord key_chord = action->Shortcut;

			ImGuiContext& g = *GImGui;
			key_chord = ImGui::FixupKeyChord(key_chord);
			ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
			if (!anyMod && (g.IO.KeyMods != mods))
				return false;

			ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
			if (key == ImGuiKey_None)
				key = ImGui::ConvertSingleModFlagToKey(mods);

			return ImGui::IsKeyDown(key);
		}

		bool IsActionHeld(std::string_view name, bool anyMod)
		{
			return IsActionHeld(Hasher(name), anyMod);
		}

		std::string_view GetActionButtonString(size_t hash)
		{
			static std::string ButtonStringCache;

			ButtonStringCache.clear();

			auto* action = GetAction(hash);
			if (!action)
				return "";

			ButtonStringCache = action->Icon + " " + action->Name;
			return ButtonStringCache;
		}

		std::string_view GetActionButtonString(std::string_view name)
		{
			return GetActionButtonString(Hasher(name));
		}

		std::string_view GetActionToolBarButtonString(size_t hash)
		{
			static std::string IconButtonStringCache;

			IconButtonStringCache.clear();

			auto* action = GetAction(hash);
			if (!action)
				return "";

			IconButtonStringCache = action->Icon;
			return IconButtonStringCache;
		}

		std::string_view GetActionToolBarButtonString(std::string_view name)
		{
			return GetActionToolBarButtonString(Hasher(name));
		}

		std::string_view GetActionShortcutString(size_t hash)
		{
            static std::string IconButtonStringCache;

            IconButtonStringCache.clear();

            auto* action = GetAction(hash);
            if (!action)
                return "";

			return ImGui::GetKeyChordName(action->Shortcut);
		}

		std::string_view GetActionShortcutString(std::string_view name)
		{
			return GetActionShortcutString(Hasher(name));
		}

		bool IsActionDefault(size_t hash)
		{
            auto* action = GetAction(hash);
            if (!action)
                return true;

			return action->DefaultShortcut == action->Shortcut;
		}

		bool SetActionKeybind(size_t hash, ImGuiKeyChord shortcut)
		{
            auto* action = GetAction(hash);
			if (!action)
				return false;

			action->Shortcut = shortcut;
			return true;
		}

		void SerializeKeybindings(rapidjson::Document& settings)
		{
			if (!settings.HasMember("keybindings"))
				settings.AddMember("keybindings", rapidjson::Value(rapidjson::kArrayType), settings.GetAllocator());

			auto value = settings.FindMember("keybindings");
			if (!value->value.IsArray())
				value->value.SetArray();

			auto& array = value->value.GetArray();
			array.Clear();

			for (auto& [id, action] : Registry)
			{
				if (IsActionDefault(id))
					continue;

				rapidjson::Value item(rapidjson::kObjectType);

				item.AddMember("action", uint64_t(id), settings.GetAllocator());
				item.AddMember("key", int(action.Shortcut), settings.GetAllocator());

				array.PushBack(item, settings.GetAllocator());
			}
		}

		void DeserializeKeybindings(rapidjson::Document& settings)
		{
			if (!settings.HasMember("keybindings"))
				return;

			auto value = settings.FindMember("keybindings");
			if (value->value.IsNull() || !value->value.IsArray())
				return;

			auto& array = value->value.GetArray();

			int size = array.Size();

			for (auto &item : array)
			{
				if (!item.IsObject() || !item.HasMember("action") || !item.HasMember("key"))
					continue;

				auto hashValue = item.FindMember("action");
				auto keyValue = item.FindMember("key");

				if (hashValue->value.IsNull() || !hashValue->value.Is<size_t>() || keyValue->value.IsNull() || !keyValue->value.Is<int>())
					continue;

				size_t hash = hashValue->value.Get<uint64_t>();
				int chord = keyValue->value.GetInt();

				ImGuiKeyChord shortcut = ImGuiKeyChord(chord);
				
                auto* action = GetAction(hash);
                if (!action)
                    continue;

				action->Shortcut = shortcut;

			}
		}
	}
}