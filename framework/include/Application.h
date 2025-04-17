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
			record.Name = T::GetDocumentTypeName();

			DocumentFactories.insert_or_assign(T::DocumentTypeID(), record);
		}

		size_t OpenDocument(size_t documentTypeID, const std::string& assetPath = "");

		template <class T>
		size_t OpenDocument(const std::string& assetPath = "")
		{
			return OpenDocument(T::DocumentTypeID(), assetPath);
		}

		bool CloseDocument(size_t documentId);

		Document* GetActiveDocument() { return ActiveDocument; }
		void OpenAssetDocument();
		size_t OpenAssetDocument(std::string_view assetPath);

		void SaveDocument(size_t documentID);
		void SaveDocumentAs(size_t documentID);

		template <class T>
		uint64_t RegisterPanel()
		{
			Panels.try_emplace(T::PanelID(), std::make_unique<T>());
			return T::PanelID();
		}

		template<class T>
		T* GetPanel()
		{
			auto itr = Panels.find(T::PanelID());
			if (itr == Panels.end())
				return nullptr;

			return static_cast<T*>(itr->second.get());
		}

		template <class T, typename ...Args>
		void ShowDialogBox(Args&&... args)
		{
			auto dlog = std::make_unique<T>(std::forward<Args>(args)...);
			ModalDialogs.emplace_back(std::move(dlog));
		}

		void Quit();

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

		virtual void OnLoadSettings(rapidjson::Document& settingsDocument) {};
		virtual void OnSaveSettings(rapidjson::Document& settingsDocument) {};

		virtual void OnRegisterDocuments() {};
		virtual void OnRegisterPanels() {};
		virtual void OnRegisterMenus() {};

		virtual void OnSetupInitalState() {};

		virtual void OnSetupMainMenuBar(MenuBar& menu) {};

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

		void RegisterDefaultMenus(MenuBar& menu);
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
			std::string Name;
		};
		std::unordered_map<uint64_t, DocumentFactoryRecord> DocumentFactories;
		std::map<uint64_t, DocumentPtr> OpenDocuments;
		std::unordered_map<uint64_t, std::unique_ptr<Panel>> Panels;
		std::deque<std::unique_ptr<Dialog>> ModalDialogs;

		std::shared_ptr<CommandContainer> WindowMenu;

		void RebuildWindowMenu();

		uint64_t UnknownDocumentFactory = 0;
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

		bool WindowMenuDirty = false;
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
