#pragma once

#include "raylib.h"

#include "imgui.h"
#include "CRC64.h"

#include "LifetimeToken.h"
#include "rapidjson/document.h"

#include <string>
#include <string_view>

namespace EditorFramework
{
	class Document;

	enum class PlanelLayoutLocation : uint8_t
	{
		Floating,
		Left,
		Right,
		Bottom,
		Max
	};

#define DEFINE_PANEL(T)\
    static uint64_t PanelID() {return Hashes::CRC64Str(#T);}\
	uint64_t GetPanelID() override {  return Hashes::CRC64Str(#T); }

	class Panel
	{
	public:
		virtual uint64_t GetPanelID() = 0;

		void Update();

		void Open() { Shown = true; }
		void Close() { Shown = false; }

		bool IsOpen() { return Shown; }

		void ForceDockSpace(ImGuiID id) { ForcedDockingID = id; }

		inline void DocumentChanged(Document* document)
		{
			OnDocumentChanged(document);
			CurrentDocument = document;
		}

		std::string_view GetImGuiName();

		std::string_view GetName() const { return Name; }
		std::string_view GetIcon() const { return Icon; }

		PlanelLayoutLocation Location = PlanelLayoutLocation::Floating;

		virtual void LoadSettings(rapidjson::Document& settings) {}
		virtual void SaveSettings(rapidjson::Document& settings) {}

	protected:
		virtual void OnUpdate() {};
		virtual void OnShow() {};
		virtual void OnDocumentChanged(Document*) {};

		Tokens::TokenSource Token;

		std::string Icon;
		std::string Name;

		Document* GetDocument() { return CurrentDocument; }

		template<class T>
		T* GetDoumentAs()
		{
			if (!CurrentDocument || CurrentDocument->GetDocumentTypeID() != T::DocumentTypeID())
				return nullptr;

			return static_cast<T*>(CurrentDocument);
		}
	private:
		bool Shown = true;

		ImGuiID ForcedDockingID = 0;

		Document* CurrentDocument = nullptr;

		Vector2 MiniumSize = { 300, 300 };

		std::string ImGuiName;
	};
}