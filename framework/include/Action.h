#pragma once

#include <string>
#include <unordered_map>

#include "imgui.h"
#include "rapidjson/document.h"

namespace EditorFramework
{
	class Action
	{
	public:
		std::string Name;
		std::string Icon;
		std::string Description;
		size_t Hash = 0;
		ImGuiKeyChord Shortcut = ImGuiKey_None;
		ImGuiKeyChord DefaultShortcut = ImGuiKey_None;
	};

	namespace ActionRegistry
	{
		extern std::unordered_map<size_t, Action> Registry;

		size_t Register(std::string_view name, std::string_view icon, std::string_view description, ImGuiKeyChord shortcut = ImGuiKey_None);

		size_t GetActionHash(std::string_view name);

		Action* GetAction(size_t hash);
		Action* GetAction(std::string_view name);

		bool IsActionTriggered(size_t hash);
		bool IsActionTriggered(std::string_view name);

		std::string_view GetActionButtonString(size_t hash);
		std::string_view GetActionButtonString(std::string_view name);
		std::string_view GetActionToolBarButtonString(size_t hash);
		std::string_view GetActionToolBarButtonString(std::string_view name);
		std::string_view GetActionShortcutString(size_t hash);
		std::string_view GetActionShortcutString(std::string_view name);

		bool IsActionDefault(size_t hash);
		bool SetActionKeybind(size_t hash, ImGuiKeyChord shortcut);

		void SerializeKeybindings(rapidjson::Document& settings);
		void DeserializeKeybindings(rapidjson::Document& settings);
	}
}