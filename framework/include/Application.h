/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you
--  wrote the original software. If you use this software in a product, an acknowledgment
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/
#pragma once

#include "raylib.h"

#include "Document.h"
#include "Panel.h"
#include "Menu.h"
#include "Dialog.h"
#include "LifetimeToken.h"

#include "rapidjson/document.h"

#include <unordered_map>
#include <map>
#include <deque>
#include <string_view>

namespace EditorFramework
{
	class Application
	{
	private:
		static Application* InstancePtr;
	public:
		static Application& GetInstance();

		Application();
		virtual ~Application();

		void SetCommandLine(char* argv[], size_t argc);
		void Run();

		template<class T>
		void RegisterDocumentType()
		{
			DocumentFactoryRecord record;
			record.Factory = T::Factory;
			record.Extension = T::FileExtension();
			record.Filter = "*." + record.Extension;

			DocumentFactories.insert_or_assign(T::DocumentTypeID(), record);
		}

		size_t OpenDocument(size_t documentTypeID);

		template <class T>
		size_t OpenDocument()
		{
			return OpenDocument(T::DocumentTypeID());
		}

		bool CloseDocument(size_t documentId);

		Document* GetActiveDocument() { return ActiveDocument; }

		void OpenAssetDocument();
		void SaveDocument(size_t documentID);
		void SaveDocumentAs(size_t documentID);

        template <class T>
        size_t RegisterPanel()
        {
			Panels.emplace_back(std::make_unique<T>());
			return Panels.size() - 1;
        }

		template <class T, typename ...Args>
		void ShowDialogBox(Args&&... args)
		{
			auto dlog = std::make_unique<T>(std::forward<Args>(args)...);
			ModalDialogs.emplace_back(std::move(dlog));
		}

		void Quit();

        MenuBar& GetMenuBar() { return MainMenu; }
		Tokens::LifetimeTokenPtr GetLifetimeToken() const { return LifeToken.GetToken(); }

		bool MouseIsInDocument();
		Vector2 MousePosInDocument();
		bool CanUseMouse();
		bool CanUseKeys();

	protected:
		virtual void OnProcessArguments(const std::vector<std::string>& args) {};
		virtual void OnStartup() {};
		virtual void OnShutdown() {};
		virtual void OnUpdate() {};
		
		virtual void OnLoadSettings(rapidjson::Document& settingsDocument){};
        virtual void OnSaveSettings(rapidjson::Document& settingsDocument){};

		virtual void OnRegisterDocuments() {};
		virtual void OnRegisterPanels() {};
		virtual void OnRegisterMenus() {};

		virtual void OnSetupInitalState() {};

		virtual void OnApplyStyle();
		virtual void SetupBackgroundTexture();

		virtual std::string_view GetWindowTitle() { return "Untitled Application"; }
		virtual std::string_view GetApplicationFolderName() { return "UntitledApp"; }

		void SetActiveDocument(size_t documentID);

		void CompletePendingSave(size_t documentId, bool save);
		void CancelPendingSave();

        MenuBar MainMenu;
        Tokens::TokenSource LifeToken;

	private:
		void Startup();
		void Update();
		void Shutdown();

		void RegisterDefaultMenus();
		void RegisterDefaultPanels();

		void LoadSettings();
		void LoadKeybindings();
		void SaveSettings();

		void ApplyWindowSettings();

		// app window settings
		static constexpr float InvalidSize = -99999999.0f;

		Vector2	WindowPos = { InvalidSize, InvalidSize };
		Vector2 WindowSize = { 1280, 800 };
		bool WindowIsMaximized = false;

		struct DocumentFactoryRecord
		{
			DocumentFactory Factory;
			std::string Extension;
			std::string Filter;
		};
		std::unordered_map<size_t, DocumentFactoryRecord> DocumentFactories;
		std::map<size_t, DocumentPtr> OpenDocuments;
		std::vector<std::unique_ptr<Panel>> Panels;
		std::deque<std::unique_ptr<Dialog>> ModalDialogs;

		CommandContainer* WindowMenu = nullptr;

		void RebuildWindowMenu();

		size_t LastDocumentId = 0;
		Document* ActiveDocument = nullptr;
		size_t FocusNextDocument = 0;

		std::vector<const char*> FileExtensions;

		RenderTexture	ActiveDocumentRenderTexture = { 0 };
		Texture BackgroundTexture = { 0 };

		bool PendingQuit = false;

		bool ResetLayouts = false;
		ImGuiID PanelLayoutDockingLocations[int(PlanelLayoutLocation::Max)] = { 0 };

		bool WantQuit = false;

		bool ShowDemoWindow = false;
		bool ShowStyleEditor = false;
		bool ShowMetricsWindow = false;

		Rectangle ContentRectangle = { 0,0,0,0 };
	};

	template<class T>
	class SingleDocumentTypeApp : public Application
	{
	public:
		void OnRegisterDocuments() override { RegisterDocumentType<T>(); }

	protected:
		void OnSetupInitalState() override { OpenDocument<T>(); }
		std::string_view GetWindowTitle()  override { return T::GetDocumentTypeName(); }
	};
}
