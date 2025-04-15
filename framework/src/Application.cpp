
#include "raylib_win32.h"

#include "Application.h"
#include "DisplayScale.h"
#include "StandardActions.h"
#include "DefaultPanels.h"
#include "KeybindingsDialog.h"

#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "tinyfiledialogs.h"

#include "rapidjson/prettywriter.h"
#include "JsonHelper.h"

#if !defined(_WIN32)
#define _stricmp stricmp
std::string GetUserFolder()
{
	return "~";
}
#else
#include <shlobj_core.h>
std::string GetUserFolder()
{
	char path[260] = { 0 };
	SHGetFolderPathA(nullptr, 0x0028, nullptr, 0, path);
	return path;
}
#endif

std::string GetUserFolder();

std::string GetAppSettingsFolder(std::string_view appFolder)
{
	std::string path = GetUserFolder();
#if defined(_WIN32)
	path += "/AppData/Local";
#endif
	path += "/";
	path += appFolder;
	MakeDirectory(path.c_str());
	return path;
}

namespace EditorFramework
{
	Application* Application::InstancePtr = nullptr;

	Application& Application::GetInstance()
	{
		_ASSERT(InstancePtr);
		return *InstancePtr;
	}

	Application::Application()
	{
		InstancePtr = this;
	}

	Application::~Application()
	{
		InstancePtr = nullptr;
	}

	void Application::SetCommandLine(char* argv[], size_t argc)
	{
		std::vector<std::string> args(argc);
		for (size_t arg = 0; arg < argc; arg++)
		{
			args.push_back(argv[arg]);
		}

		OnProcessArguments(args);
	}

	void Application::Run()
	{
		Startup();

		while (!WantQuit)
		{
			Update();

			if (WindowShouldClose())
				Quit();

			//TODO, check if we can quit
		}

		Shutdown();
	}

	void Application::Quit()
	{
		if (PendingQuit)
			return;

		while (!OpenDocuments.empty())
		{
			if (!CloseDocument(OpenDocuments.begin()->first))
			{
				PendingQuit = true;
				return;
			}
		}

		WantQuit = true;
	}

	void Application::Startup()
	{
		// set DPI aware outside so we can get the display scale first
		SetDPIAware();

		LogSink::Init();

		LoadSettings();

		// do not set HI DPI in flags, we are going to manage that ourselves
		SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN);

		Vector2 windowSize = WindowSize;
		windowSize = ScaleToDPI(windowSize);

		InitWindow(int(windowSize.x), int(windowSize.y), GetWindowTitle().data());
		ApplyWindowSettings();

		ClearWindowState(FLAG_WINDOW_HIDDEN);

		SetExitKey(KEY_NULL);

		rlImGuiSetup(true);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (!FileExists(ImGui::GetIO().IniFilename))
			ResetLayouts = true;

		OnApplyStyle();
		SetupBackgroundTexture();

		OnStartup();

		OnRegisterDocuments();

		// cache the document extensions
		if (FileExtensions.empty())
		{
			for (auto& [id, factory] : DocumentFactories)
			{
				if (!factory.Extension.empty())
					FileExtensions.push_back(factory.Filter.c_str());
			}
		}

		OnRegisterPanels();

		RegisterDefaultPanels();

		RegisterStandardActions();

		RegisterDefaultMenus(MainMenu);
		WindowMenu = MainMenu.AddSubItem("Window", "", 610);

		OnRegisterMenus();

		LoadKeybindings();

		OnSetupInitalState();
	}

	void Application::ApplyWindowSettings()
	{
		if (WindowPos.x > InvalidSize)
		{
			// ensure that the window is on at least one screen
			bool apply = false;
			for (int i = 0; i < GetMonitorCount(); i++)
			{
				Rectangle rect = { GetMonitorPosition(i).x, GetMonitorPosition(i).y, float(GetMonitorWidth(i)), float(GetMonitorHeight(i)) };

				if (CheckCollisionPointRec(WindowPos, rect))
					apply = true;
			}

			if (apply)
				SetWindowPosition(int(WindowPos.x), int(WindowPos.y));
		}

		if (WindowIsMaximized)
			MaximizeWindow();

	}

	void Application::Update()
	{
		RebuildWindowMenu();

		BeginDrawing();
		ClearBackground(DARKGRAY);

		if (IsTextureValid(BackgroundTexture))
			DrawTextureRec(BackgroundTexture, Rectangle{ 0,0,float(GetScreenWidth()),float(GetScreenHeight()) }, Vector2Zeros, WHITE);

		OnUpdate();

		rlImGuiBegin();

		Rectangle contentArea{ 0,0,float(GetScreenWidth()), float(GetScreenHeight()) };

		MenuBar* barToUse = &MainMenu;
		if (ActiveDocument)
			barToUse = &ActiveDocument->GetDocumentMenu();

		barToUse->Show(true);
		barToUse->ProcessShortcuts();

		float menuOffset = barToUse->GetHeight() + ImGui::GetStyle().FrameBorderSize;
		{
			ImGui::SetNextWindowPos(ImVec2(0, menuOffset));
			auto size = ImVec2(float(GetScreenWidth()), float(GetScreenHeight()));
			ImGui::SetNextWindowSize(size);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			ImGuiWindowFlags frameFlags = ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoDocking
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoBringToFrontOnFocus
				| ImGuiWindowFlags_NoFocusOnAppearing
				| ImGuiWindowFlags_NoNavFocus
				| ImGuiWindowFlags_NoNavInputs
				| ImGuiWindowFlags_NoResize;

			bool show = ImGui::Begin("###MAIN_FRAME", nullptr, frameFlags);
			ImGui::PopStyleVar();

			// build a dockspace in the area we have left but don't let anyone dock to the central node, we want to leave that open for the document content and toolbar
			auto contentSize = ImGui::GetContentRegionAvail();
			auto dockId = ImGui::DockSpace(ImGui::GetID("###MAIN_DOCKSPACE"), contentSize, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);

			ImGui::End();

			if (ResetLayouts)
			{
				ImGui::DockBuilderRemoveNodeDockedWindows(dockId);
				ImGui::DockBuilderRemoveNodeChildNodes(dockId);

				ImGuiID centralNode;

				PanelLayoutDockingLocations[int(PlanelLayoutLocation::Floating)] = ImGuiID(-1);

				// split the central node left and right
				ImGui::DockBuilderSplitNode(dockId, ImGuiDir::ImGuiDir_Left, 0.25f, &PanelLayoutDockingLocations[int(PlanelLayoutLocation::Left)], &centralNode);

				// split the new central node right and left
				ImGui::DockBuilderSplitNode(centralNode, ImGuiDir::ImGuiDir_Right, 0.25f, &PanelLayoutDockingLocations[int(PlanelLayoutLocation::Right)], &centralNode);

				// split the new central node top and bottom
				ImGui::DockBuilderSplitNode(centralNode, ImGuiDir::ImGuiDir_Down, 0.25f, &PanelLayoutDockingLocations[int(PlanelLayoutLocation::Bottom)], &centralNode);
			}

			auto node = ImGui::DockBuilderGetNode(dockId);

			auto forcedId = dockId;

			// if we have a central node, draw the toolbar and overlay
			if (node && node->CentralNode)
			{
				forcedId = node->CentralNode->ID;
			}

			size_t documentToClose = 0;

			for (auto& [id, doc] : OpenDocuments)
			{
				bool open = true;

				ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
				ImGui::SetNextWindowDockID(forcedId, ImGuiCond_Always);

				if (id == FocusNextDocument)
				{
					ImGui::SetNextWindowFocus();
					FocusNextDocument = 0;
				}

				const char* name = TextFormat("%s###%s_%d", doc->GetDocumentName().data(), doc->GetDocumentName().data(), id);

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoBringToFrontOnFocus
					| ImGuiWindowFlags_NoFocusOnAppearing
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoNavFocus
					| ImGuiWindowFlags_NoNavInputs;
				if (doc->IsDirty())
					flags |= ImGuiWindowFlags_UnsavedDocument;

				bool documentVis = ImGui::Begin(name, &open, flags);
				ImGui::PopStyleVar(1);

				if (!open)
					documentToClose = id;

				if (documentVis)
				{
					if (documentVis && doc.get() != ActiveDocument)
						SetActiveDocument(id);

					auto contentArea = ImGui::GetContentRegionAvail();

					ContentRectangle.x = ImGui::GetCursorScreenPos().x;
					ContentRectangle.y = ImGui::GetCursorScreenPos().y;

					ContentRectangle.width = contentArea.x;
					ContentRectangle.height = contentArea.y;

					doc->OnUpdate(int(contentArea.x), int(contentArea.y));

					if (!IsRenderTextureValid(ActiveDocumentRenderTexture) || int(contentArea.x) != ActiveDocumentRenderTexture.texture.width || int(contentArea.y) != ActiveDocumentRenderTexture.texture.height)
					{
						if (IsRenderTextureValid(ActiveDocumentRenderTexture))
							UnloadRenderTexture(ActiveDocumentRenderTexture);

						ActiveDocumentRenderTexture = LoadRenderTexture(int(contentArea.x), int(contentArea.y));
					}

					BeginTextureMode(ActiveDocumentRenderTexture);
					auto color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
					ClearBackground(Color{ uint8_t(color.x * 255), uint8_t(color.y * 255), uint8_t(color.z * 255), uint8_t(color.w * 255) });
					doc->OnShowContent(int(contentArea.x), int(contentArea.y));
					EndTextureMode();

					auto cursor = ImGui::GetCursorPos();
					rlImGuiImageRenderTexture(&ActiveDocumentRenderTexture);

					ImGui::SetItemAllowOverlap();

					ImGui::SetCursorPos(cursor);

					auto* docToolbar = doc->GetToolbar();
					if (docToolbar)
						docToolbar->Show();

					doc->OnShowUI();
				}
				ImGui::End();
			}
			if (documentToClose != 0)
			{
				CloseDocument(documentToClose);
			}
		}

		for (auto& [id, panel] : Panels)
		{
			if (ResetLayouts)
				panel->ForceDockSpace(PanelLayoutDockingLocations[int(panel->Location)]);

			panel->Update();
		}

		if (ShowDemoWindow)
			ImGui::ShowDemoWindow(&ShowDemoWindow);

		if (ShowStyleEditor)
		{
			if (ImGui::Begin("ImGui Style Editor", &ShowDemoWindow))
				ImGui::ShowStyleEditor();
			ImGui::End();
		}

		if (ShowMetricsWindow)
			ImGui::ShowMetricsWindow(&ShowMetricsWindow);

		if (!ModalDialogs.empty())
		{
			if (ModalDialogs.front()->Process())
				ModalDialogs.pop_front();
		}

		ResetLayouts = false;

		// Draw ImGui over the top of the background
		rlImGuiEnd();

		EndDrawing();
	}

	void Application::Shutdown()
	{
		SaveSettings();

		if (IsTextureValid(BackgroundTexture))
			UnloadTexture(BackgroundTexture);

		if (IsRenderTextureValid(ActiveDocumentRenderTexture))
			UnloadRenderTexture(ActiveDocumentRenderTexture);

		rlImGuiShutdown();
		OnShutdown();
		CloseWindow();
	}

	void Application::LoadSettings()
	{
		std::string settingsPath = GetAppSettingsFolder(GetApplicationFolderName());
		settingsPath += "/settings.json";

		rapidjson::Document document;

		if (FileExists(settingsPath.c_str()))
		{
			auto text = LoadFileText(settingsPath.c_str());
			document.Parse(text);
			UnloadFileText(text);
		}
		else
		{
			document.SetObject();
		}

		// app settings for window
		WindowSize.x = JSONHelper::GetValue<float>(document, "WindowSizeX", 1280);
		WindowSize.y = JSONHelper::GetValue<float>(document, "WindowSizeY", 800);
		WindowPos.x = JSONHelper::GetValue<float>(document, "WindowPosX", InvalidSize);
		WindowPos.y = JSONHelper::GetValue<float>(document, "WindowPosY", -InvalidSize);
		WindowIsMaximized = JSONHelper::GetValue<bool>(document, "WindowIsMaximized", false);

		OnLoadSettings(document);
	}

	void Application::LoadKeybindings()
	{
		std::string settingsPath = GetAppSettingsFolder(GetApplicationFolderName());
		settingsPath += "/settings.json";

		rapidjson::Document document;

		if (FileExists(settingsPath.c_str()))
		{
			auto text = LoadFileText(settingsPath.c_str());
			document.Parse(text);
			UnloadFileText(text);
		}
		else
		{
			document.SetObject();
		}

		ActionRegistry::DeserializeKeybindings(document);
	}

	void Application::SaveSettings()
	{
		std::string settingsPath = GetAppSettingsFolder(GetApplicationFolderName());
		settingsPath += "/settings.json";

		rapidjson::Document document;

		if (FileExists(settingsPath.c_str()))
		{
			auto text = LoadFileText(settingsPath.c_str());
			document.Parse(text);
			UnloadFileText(text);
		}
		else
		{
			document.SetObject();
		}
		WindowSize.x = GetScreenWidth() / GetWindowScaleDPI().x;
		WindowSize.y = GetScreenHeight() / GetWindowScaleDPI().y;

		WindowPos.x = GetWindowPosition().x;
		WindowPos.y = GetWindowPosition().y;

		WindowIsMaximized = IsWindowMaximized();

		if (!WindowIsMaximized)
		{
			JSONHelper::SetOrCreateValue(document, "WindowSizeX", WindowSize.x, document);
			JSONHelper::SetOrCreateValue(document, "WindowSizeY", WindowSize.y, document);
		}
		JSONHelper::SetOrCreateValue(document, "WindowPosX", WindowPos.x, document);
		JSONHelper::SetOrCreateValue(document, "WindowPosY", WindowPos.y, document);
		JSONHelper::SetOrCreateValue(document, "WindowIsMaximized", WindowIsMaximized, document);

		ActionRegistry::SerializeKeybindings(document);

		OnSaveSettings(document);

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);

		SaveFileText(settingsPath.c_str(), (char*)buffer.GetString());
	}

	void Application::RegisterDefaultPanels()
	{
		RegisterPanel<LogPanel>();
	}

	void Application::RegisterDefaultMenus(MenuBar& menu)
	{
		// file menu
		auto fileMenu = menu.AddSubItem("File", "", 10);

		auto openGroup = fileMenu->AddGroup("Open", ICON_FA_FOLDER_OPEN, 10);

		auto newAsset = openGroup->AddSubItem("New Asset", ICON_FA_FILE_CIRCLE_PLUS, 0);

		int index = 0;
		for (auto& [typeId, factory] : DocumentFactories)
		{
			index++;
			newAsset->AddItem<StateMenuCommand>(index,
				"",
				factory.Name,
				[&typeId, this](CommandContextSet*) {OpenDocument(typeId); },
				nullptr);
		}

		openGroup->AddItem<ActionCommandItem>(10, OpenAction, [this](float, CommandContextSet*) { OpenAssetDocument(); });

		auto closeGroup = fileMenu->AddGroup("Close", ICON_FA_WINDOW_MAXIMIZE, 20);
		closeGroup->AddItem<ActionCommandItem>(0,
			CloseAction,
			[this](float, CommandContextSet*) {CloseDocument(ActiveDocument->GetDocumentID()); },
			[this](CommandContextSet*) { return ActiveDocument != nullptr; });

		closeGroup->AddItem<ActionCommandItem>(10,
			CloseAllAction,
			[this](float, CommandContextSet*) {},
			[this](CommandContextSet*) { return OpenDocuments.size() > 0; });

		closeGroup->AddItem<ActionCommandItem>(20,
			CloseAllButAction,
			[this](float, CommandContextSet*) {},
			[this](CommandContextSet*) { return OpenDocuments.size() > 1; });

		auto& saveGroup = fileMenu->AddGroup("Save", ICON_FA_FLOPPY_DISK, 30);

		saveGroup->AddItem<ActionCommandItem>(20,
			SaveAction,
			[this](float, CommandContextSet*) { SaveDocument(ActiveDocument->GetDocumentID()); },
			[this](CommandContextSet*) { return ActiveDocument != nullptr && (ActiveDocument->GetAssetPath().empty() || ActiveDocument->IsDirty()); });

		saveGroup->AddItem<ActionCommandItem>(20,
			SaveAsAction,
			[this](float, CommandContextSet*) {SaveDocumentAs(ActiveDocument->GetDocumentID()); },
			[this](CommandContextSet*) { return ActiveDocument != nullptr && (ActiveDocument->GetAssetPath().empty() || ActiveDocument->IsDirty()); });

		auto prefsGroup = fileMenu->AddGroup("Preferences", ICON_FA_LIST_CHECK, 400);
		//	prefsGroup.AddItem<ActionCommandItem>(0, PreferencesAction, [this](float) { });
		prefsGroup->AddItem<ActionCommandItem>(10, ShortcutKeysAction, [this](float, CommandContextSet*) {ShowDialogBox<KeybindingDialog>(); });

		auto& endGroup = fileMenu->AddGroup("Exit", "", 1000);
		endGroup->AddItem<ActionCommandItem>(1000, ExitAction, [this](float, CommandContextSet*) { Quit(); });

		// programmer menu
		auto programmer = menu.AddSubItem("Programmer", "", 500);
		auto imGuiGroup = programmer->AddGroup("ImGui", ICON_FA_PEN_RULER, 10);

		imGuiGroup->AddItem<ActionCommandItem>(0, ImGuiItemPickerAction, [this](float, CommandContextSet*) { ImGui::DebugStartItemPicker(); });

		imGuiGroup->AddItem<ActionCommandItem>(0, ImGuiDemoAction, [this](float, CommandContextSet*) { ShowDemoWindow = !ShowDemoWindow; },
			nullptr,
			[this](CommandContextSet*) {return ShowDemoWindow; });

		imGuiGroup->AddItem<ActionCommandItem>(0, ImGuiStyleAction, [this](float, CommandContextSet*) { ShowStyleEditor = !ShowStyleEditor; },
			nullptr,
			[this](CommandContextSet*) {return ShowStyleEditor; });

		imGuiGroup->AddItem<ActionCommandItem>(0, ImGuiMetricsAction, [this](float, CommandContextSet*) { ShowMetricsWindow = !ShowMetricsWindow; },
			nullptr,
			[this](CommandContextSet*) {return ShowMetricsWindow; });

		// Panel menu
		auto panelMenu = menu.AddSubItem("Panels", "", 600);
		auto panelsGroup = panelMenu->AddGroup("Panels", ICON_FA_WINDOW_RESTORE, 10);

		index = 0;
		for (auto& [id, panel] : Panels)
		{
			index++;
			panelsGroup->AddItem<StateMenuCommand>(index,
				panel->GetIcon(),
				panel->GetName(),
				[&panel](CommandContextSet*) { if (panel->IsOpen()) panel->Close(); else panel->Open(); },
				[&panel](CommandContextSet*) { return panel->IsOpen(); });
		}

		auto& panelManagementGroup = panelMenu->AddGroup("Management", ICON_FA_TABLE_COLUMNS, Panels.size() + 10);

		panelManagementGroup->AddItem<ActionCommandItem>(0, ResetLayoutAction, [this](float, CommandContextSet*) { ResetLayouts = true; });

		// Window menu
		if (WindowMenu != nullptr)
		{
			menu.InsertContainer(610, WindowMenu);
		}

		OnSetupMainMenuBar(menu);
	}

	void Application::RebuildWindowMenu()
	{
		if (!WindowMenu || !WindowMenuDirty)
			return;

		WindowMenuDirty = false;
		WindowMenu->Contents.clear();

		for (auto& [id, doc] : OpenDocuments)
		{
			WindowMenu->AddItem<StateMenuCommand>(id,
				"",
				doc->GetDocumentName(),
				[&id, this](CommandContextSet*) { FocusNextDocument = id; },
				[&id, this](CommandContextSet*) { return ActiveDocument && ActiveDocument->GetDocumentID() == id; });
		}
	}

	size_t Application::OpenDocument(size_t documentTypeID, const std::string& assetPath)
	{
		auto factoryItr = DocumentFactories.find(documentTypeID);
		if (factoryItr == DocumentFactories.end())
			return 0;

		LastDocumentId++;
		OpenDocuments.try_emplace(LastDocumentId, std::move(factoryItr->second.Factory()));

		auto* doc = OpenDocuments[LastDocumentId].get();

		doc->OnDirty.Add([this](const Document& doc)
			{
				if (ActiveDocument && ActiveDocument->GetDocumentID() == doc.GetDocumentID())
				{
					// refresh the title bar
					SetActiveDocument(ActiveDocument->GetDocumentID());
				}
			}, GetLifetimeToken());

		doc->HostApp = this;
		doc->Created(LastDocumentId);

		if (assetPath.empty())
			doc->CreateAsset();
		else
			doc->OpenAsset(assetPath);

		RegisterDefaultMenus(doc->GetDocumentMenu());
		doc->OnSetupMenu();

		SetActiveDocument(LastDocumentId);

		WindowMenuDirty = true;

		return LastDocumentId;
	}

	bool Application::CloseDocument(size_t documentId)
	{
		bool wasActiveDoc = ActiveDocument != nullptr && ActiveDocument->GetDocumentID() == documentId;

		auto itr = OpenDocuments.find(documentId);

		if (itr == OpenDocuments.end())
			return true;

		if (itr->second->IsDirty())
		{
			size_t id = itr->first;
			ShowDialogBox<SaveDirtyFileDialog>(itr->second->GetDocumentName().data(),
				[this, id](Dialog&) {CompletePendingSave(id, true); },
				[this, id](Dialog&) {CompletePendingSave(id, false); },
				[this](Dialog&) {CancelPendingSave(); });
			return false;
		}

		if (wasActiveDoc)
			itr->second->OnDeactivated();

		OpenDocuments.erase(itr);

		if (wasActiveDoc)
		{
			ActiveDocument = nullptr;

			if (OpenDocuments.size())
				SetActiveDocument(OpenDocuments.begin()->second->GetDocumentID());
			else
				SetActiveDocument(0);
		}
		WindowMenuDirty = true;

		return true;
	}

	void Application::CompletePendingSave(size_t documentID, bool save)
	{
		bool wasActiveDocument = ActiveDocument && ActiveDocument->GetDocumentID() == documentID;

		auto itr = OpenDocuments.find(documentID);
		if (itr != OpenDocuments.end())
		{
			if (save)
				SaveDocument(documentID);
			else
				itr->second->SetClean();

			if (wasActiveDocument)
				itr->second->OnDeactivated();
			OpenDocuments.erase(itr);
		}

		if (wasActiveDocument)
		{
			ActiveDocument = nullptr;
			if (!OpenDocuments.empty())
				SetActiveDocument(OpenDocuments.begin()->first);
			else
				SetActiveDocument(0);
		}
		WindowMenuDirty = true;

		if (PendingQuit)
		{
			while (!OpenDocuments.empty())
			{
				if (!CloseDocument(OpenDocuments.begin()->first))
				{
					PendingQuit = true;
					return;
				}
			}

			PendingQuit = false;
			Quit();
		}
	}

	void Application::CancelPendingSave()
	{
		PendingQuit = false;
	}

	void Application::OpenAssetDocument()
	{
		auto fileToOpen = tinyfd_openFileDialog("Open file...", nullptr, int(FileExtensions.size()), &FileExtensions[0], nullptr, false);

		if (!fileToOpen)
			return;

		auto extension = GetFileExtension(fileToOpen);
		extension++;

		size_t idToUse = 0;
		for (auto& [id, factory] : DocumentFactories)
		{
			if (_stricmp(factory.Extension.c_str(), extension) == 0)
			{
				idToUse = id;
				break;
			}
		}

		if (idToUse == 0)
			idToUse = DocumentFactories.begin()->first;

		if (idToUse != 0)
		{
			auto documentId = OpenDocument(idToUse, fileToOpen);
		}
	}

	void Application::SaveDocument(size_t documentID)
	{
		auto itr = OpenDocuments.find(documentID);

		if (itr == OpenDocuments.end())
			return;

		if (!itr->second->GetAssetPath().empty())
			itr->second->SaveAsset();
		else
			SaveDocumentAs(documentID);
	}

	void Application::SaveDocumentAs(size_t documentID)
	{
		auto itr = OpenDocuments.find(documentID);

		if (itr == OpenDocuments.end())
			return;

		std::vector<const char*> extensions;

		extensions.push_back(DocumentFactories[itr->second->GetDocumentTypeID()].Filter.c_str());
		std::string desiredExtension = "." + DocumentFactories[itr->second->GetDocumentTypeID()].Extension;

		std::string initalFileName(itr->second->GetAssetPath());
		if (initalFileName.empty())
			initalFileName = "*" + desiredExtension;

		const char* fileName = tinyfd_saveFileDialog("Save file as...",
			initalFileName.c_str(),
			int(extensions.size()),
			&extensions[0],
			nullptr);

		// if they cancel, and we have a pending quit, then stop that quit
		if (!fileName)
		{
			if (PendingQuit)
				PendingQuit = false;

			return;
		}

		std::string actualFileName = fileName;

		std::string extension;
		const char* ext = GetFileExtension(fileName);
		if (ext)
			extension = ext;

		if (extension != desiredExtension)
			actualFileName += desiredExtension;

		itr->second->SaveAsAsset(actualFileName.c_str());
	}

	void Application::SetActiveDocument(size_t documentID)
	{
		if (ActiveDocument && ActiveDocument->GetDocumentID() == documentID)
			return;

		if (ActiveDocument)
			ActiveDocument->OnDeactivated();

		ActiveDocument = nullptr;

		auto docItr = OpenDocuments.find(documentID);
		if (docItr != OpenDocuments.end())
		{
			ActiveDocument = OpenDocuments[documentID].get();
			ActiveDocument->OnActivated();
		}

		char prefix = ' ';
		if (ActiveDocument && ActiveDocument->IsDirty())
			prefix = '*';

		if (ActiveDocument)
			SetWindowTitle(TextFormat("%c%s - %s", prefix, ActiveDocument->GetDocumentName().data(), GetWindowTitle().data()));
		else
			SetWindowTitle(GetWindowTitle().data());

		for (auto& [id, panel] : Panels)
		{
			panel->DocumentChanged(ActiveDocument);
		}
	}

	bool Application::MouseIsInDocument()
	{
		return CheckCollisionPointRec(::GetMousePosition(), ContentRectangle);
	}

	Vector2 Application::MousePosInDocument()
	{
		return GetMousePosition() - Vector2{ ContentRectangle.x,ContentRectangle.y };
	}

	bool Application::CanUseMouse()
	{
		return !ImGui::GetIO().WantCaptureMouse;
	}

	bool Application::CanUseKeys()
	{
		return !ImGui::GetIO().WantCaptureKeyboard;
	}

	void Application::SetupBackgroundTexture()
	{
		Image img = GenImageChecked(ScaleToDPI(128), ScaleToDPI(128), ScaleToDPI(32), ScaleToDPI(32), Color{ 80, 80, 80, 255 }, Color{ 75, 75, 80, 255 });
		BackgroundTexture = LoadTextureFromImage(img);
		UnloadImage(img);
	}

	void Application::OnApplyStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Base colors for a pleasant and modern dark theme with dark accents
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);                  // Light grey text for readability
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.54f, 1.00f);          // Subtle grey for disabled text
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);              // Dark background with a hint of blue
		colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);               // Slightly lighter for child elements
		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);               // Popup background
		colors[ImGuiCol_Border] = ImVec4(0.28f, 0.29f, 0.30f, 0.60f);                // Soft border color
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);          // No border shadow
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);               // Frame background
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);        // Frame hover effect
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Active frame background
		colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);               // Title background
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);         // Active title background
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);      // Collapsed title background
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);             // Menu bar background
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);           // Scrollbar background
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Dark accent for scrollbar grab
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.30f, 0.32f, 1.00f);  // Scrollbar grab hover
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);   // Scrollbar grab active
		colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Dark blue checkmark
		colors[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Dark blue slider grab
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);      // Active slider grab
		colors[ImGuiCol_Button] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Dark blue button
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Button hover effect
		colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active button
		colors[ImGuiCol_Header] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Header color similar to button
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Header hover effect
		colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active header
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);             // Separator color
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for separator
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);       // Active separator
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Resize grip
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);     // Hover effect for resize grip
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.44f, 0.54f, 0.64f, 1.00f);      // Active resize grip
		colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);                   // Inactive tab
		colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);            // Hover effect for tab
		colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);             // Active tab color
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);          // Unfocused tab
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);    // Active but unfocused tab
		colors[ImGuiCol_DockingPreview] = ImVec4(0.24f, 0.34f, 0.44f, 0.70f);        // Docking preview
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);        // Empty docking background
		colors[ImGuiCol_PlotLines] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Plot lines
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for plot lines
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);         // Histogram color
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);  // Hover effect for histogram
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);         // Table header background
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);     // Strong border for tables
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.24f, 0.25f, 0.26f, 1.00f);      // Light border for tables
		colors[ImGuiCol_TableRowBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);            // Table row background
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);         // Alternate row background
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.34f, 0.44f, 0.35f);        // Selected text background
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.46f, 0.56f, 0.66f, 0.90f);        // Drag and drop target
		colors[ImGuiCol_NavHighlight] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);          // Navigation highlight
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f); // Windowing highlight
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);     // Dim background for windowing
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.14f, 0.14f, 0.16f, 0.5f);      // Dim background for modal windows

		// Style adjustments
		style.WindowRounding = 8.0f;    // Softer rounded corners for windows
		style.FrameRounding = 4.0f;     // Rounded corners for frames
		style.ScrollbarRounding = 6.0f; // Rounded corners for scrollbars
		style.GrabRounding = 4.0f;      // Rounded corners for grab elements
		style.ChildRounding = 4.0f;     // Rounded corners for child windows

		style.WindowTitleAlign = ImVec2(0.50f, 0.50f); // Centered window title
		style.WindowPadding = ImVec2(10.0f, 10.0f);    // Comfortable padding
		style.FramePadding = ImVec2(6.0f, 4.0f);       // Frame padding
		style.ItemSpacing = ImVec2(8.0f, 8.0f);        // Item spacing
		style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);   // Inner item spacing
		style.IndentSpacing = 22.0f;                   // Indentation spacing

		style.ScrollbarSize = 16.0f; // Scrollbar size
		style.GrabMinSize = 10.0f;   // Minimum grab size

		style.AntiAliasedLines = true; // Enable anti-aliased lines
		style.AntiAliasedFill = true;  // Enable anti-aliased fill

		style.WindowMenuButtonPosition = ImGuiDir_Right;
	}
}
