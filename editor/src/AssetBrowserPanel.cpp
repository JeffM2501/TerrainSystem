#pragma warning (disable : 4996)

#include "AssetBrowserPanel.h"
#include "AssetManager.h"
#include "AssetDatabase.h"
#include "extras/IconsFontAwesome6.h"
#include "imgui_internal.h"
#include "DisplayScale.h"
#include "Application.h"
#include "JsonHelper.h"

using namespace EditorFramework;
using namespace AttributeTypes;
using namespace AssetSystem;
using namespace Types;

static std::unordered_map<std::string, std::string> ExtensionMap;

static char* stristr(const char* str1, const char* str2)
{
	const char* p1 = str1;
	const char* p2 = str2;
	const char* r = *p2 == 0 ? str1 : 0;

	while (*p1 != 0 && *p2 != 0)
	{
		if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
		{
			if (r == 0)
			{
				r = p1;
			}

			p2++;
		}
		else
		{
			p2 = str2;
			if (r != 0)
			{
				p1 = r + 1;
			}

			if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
			{
				r = p1;
				p2++;
			}
			else
			{
				r = 0;
			}
		}

		p1++;
	}

	return *p2 == 0 ? (char*)r : 0;
}

namespace AssetItems
{
	void FolderInfo::PopulateChildren()
	{
		ImVec4 folderColor = ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered];

		auto folders = LoadDirectoryFiles(FullPath.c_str());

		for (unsigned int i = 0; i < folders.count; i++)
		{
			if (DirectoryExists(folders.paths[i]))
			{
				const char* name = GetFileNameWithoutExt(folders.paths[i]);
				if (!name || *name == '.')
					continue;

				FolderInfo& child = Children.emplace_back();
				child.FullPath = folders.paths[i];
				child.Name = name;
				child.RelativePath = RelativePath + "/" + name;
				child.Parent = this;
				child.Tint = folderColor;
				child.Icon = ICON_FA_FOLDER;
				child.PopulateChildren();
			}
		}
		UnloadDirectoryFiles(folders);
	}

	ViewableItem* AssetContainer::Reset()
	{
		FileItr = Files.begin();
		if (Folder)
			FolderItr = Folder->Children.begin();
		if (FileItr != Files.end())
			return &(*FileItr);
		if (Folder && FolderItr != Folder->Children.end())
			return &(*FolderItr);

		return nullptr;
	}

	size_t AssetContainer::Count()
	{
		return Files.size() + Folder->Children.size();
	}

	ViewableItem* AssetContainer::Next()
	{
		if (FileItr != Files.end())
		{
			FileItr++;
			if (FileItr != Files.end())
				return &(*FileItr);
			else
			{
				if (UseFolders && Folder && FolderItr != Folder->Children.end())
					return &(*FolderItr);
				else
					return nullptr;
			}
		}

		if (UseFolders && Folder && FolderItr != Folder->Children.end())
		{
			FolderItr++;
			if (FolderItr != Folder->Children.end())
				return &(*FolderItr);
		}

		return nullptr;
	}

	std::string_view GetFileIcon(const char* filename)
	{
		const char* ext = GetFileExtension(filename);

		if (ext != nullptr)
		{
			if (stricmp(ext, ".png") == 0)
				return ICON_FA_FILE_IMAGE;

			if (stricmp(ext, ".wav") == 0 || stricmp(ext, ".mp3") == 0 || stricmp(ext, ".oog") == 0)
				return ICON_FA_FILE_AUDIO;

			if (stricmp(ext, ".ttf") == 0 || stricmp(ext, ".otf") == 0 || stricmp(ext, ".fnt") == 0)
				return ICON_FA_FONT;

			if (stricmp(ext, ".txt") == 0 || stricmp(ext, ".md") == 0)
				return ICON_FA_FILE_LINES;

			if (stricmp(ext, ".lua") == 0 || stricmp(ext, ".c") == 0 || stricmp(ext, ".h") == 0 || stricmp(ext, ".cpp") == 0)
				return ICON_FA_FILE_CODE;
		}
		return ICON_FA_FILE;
	}

	void AssetFolderPopulator::Populate(AssetContainer& container, FolderInfo* root)
	{
		container.Folder = root;
		container.UseFolders = true;

		if (root == nullptr)
			return;

		auto files = LoadDirectoryFiles(container.Folder->FullPath.c_str());

		for (unsigned int i = 0; i < files.count; i++)
		{
			if (DirectoryExists(files.paths[i]))
				continue;

			const char* name = GetFileName(files.paths[i]);
			if (!name)
				continue;

			FileInfo& file = container.Files.emplace_back();
			file.FullPath = files.paths[i];
			file.Name = name;
			file.Shown = *name != '.';

			auto itr = ExtensionMap.find(GetFileExtension(name));
			if (itr != ExtensionMap.end())
				file.Icon = itr->second;
			else
				file.Icon = GetFileIcon(name);
		}

		UnloadDirectoryFiles(files);
	}

	void AssetSearchPopulator::Populate(AssetContainer& container, FolderInfo* root, const char* search)
	{
		container.Folder = root;
		container.UseFolders = false;

		if (root == nullptr)
			return;

		auto files = LoadDirectoryFilesEx(container.Folder->FullPath.c_str(), search, true);

		for (unsigned int i = 0; i < files.count; i++)
		{
			if (DirectoryExists(files.paths[i]))
				continue;

			const char* name = GetFileName(files.paths[i]);
			if (!name)
				continue;

			FileInfo& file = container.Files.emplace_back();
			file.FullPath = files.paths[i];
			file.Name = name;
			file.Shown = *name != '.';
			auto itr = ExtensionMap.find(GetFileExtension(file.FullPath.c_str()));
			if (itr != ExtensionMap.end())
				file.Icon = itr->second;
			else
				file.Icon = GetFileIcon(name);
		}

		UnloadDirectoryFiles(files);
	}
}

AssetBrowserPanel::AssetBrowserPanel()
	: GridView(ScaleToDPI(80.0f))
{
	Icon = ICON_FA_FOLDER;
	Name = "Asset Browser";

	Location = PlanelLayoutLocation::Bottom;


	for (auto& assetTypeInfo : AssetSystem::AssetTypeDatabase::GetAssetTypes())
	{
		ExtensionMap["." + assetTypeInfo.Extension] = assetTypeInfo.Icon;
	}

	RebuildFolderTree();
	SetCurrentFolder(&FolderRoot);

	CurrentView = &ListView;

	AssetManager::AssetRootChanged.Add([this](const std::string&)
		{
			RebuildFolderTree();
			ApplyCurrentFolder(&FolderRoot);
			//SetCurrentFolder(&FolderRoot);
		}, Token.GetToken());
}


void AssetBrowserPanel::RebuildFolderTree()
{
	FolderRoot.Children.clear();

	FolderRoot.FullPath = AssetManager::ToFileSystemPath("");
	FolderRoot.Name = "Root";
	FolderRoot.RelativePath = "";
	FolderRoot.Parent = nullptr;
	FolderRoot.Icon = ICON_FA_SERVER;
	FolderRoot.ForceOpenNextFrame = true;
	FolderRoot.PopulateChildren();
}

void AssetBrowserPanel::OnShow()
{
	ShowHeader();

	ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV;

	if (ImGui::BeginTable("AssetBrowserTab", 2, flags, ImGui::GetContentRegionAvail()))
	{
		ImGui::TableSetupColumn("FolderView", ImGuiTableColumnFlags_None, 0.25f);
		ImGui::TableSetupColumn("AssetView", ImGuiTableColumnFlags_None, 0.75f);

		ImGui::TableNextRow(ImGuiTableRowFlags_None);
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild("FolderList", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_None))
		{
			ShowFolderTree();
		}
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		if (CurrentFolderContents.Folder)
		{
			if (ImGui::BeginChild("FilesList", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_None))
			{
				ShowFilePane();
			}
			ImGui::EndChild();
		}
		ImGui::EndTable();
	}

	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
	{
		if (IsMouseButtonPressed(MOUSE_BUTTON_SIDE))
			NavigateBack();

		if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA))
			NavigateForward();
	}
}

void AssetBrowserPanel::ShowFolderTree()
{
	CurrentFolderContents.OpenTreeFolders.clear();
	ShowFolderTreeNode(FolderRoot);
}

bool AssetBrowserPanel::ShowFolderTreeNode(AssetItems::FolderInfo& info)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (info.Children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (CurrentFolderContents.Folder == &info)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (info.ForceOpenNextFrame)
		ImGui::SetNextItemOpen(true);

	bool open = ImGui::TreeNodeEx(info.Name.c_str(), flags, "%s %s", info.Icon.c_str(), info.Name.c_str());

	if (open)
		CurrentFolderContents.OpenTreeFolders.push_back(&info);

	if (info.ForceOpenNextFrame && CurrentFolderContents.Folder == &info)
		ImGui::ScrollToItem(ImGuiScrollFlags_KeepVisibleCenterY);

	info.ForceOpenNextFrame = false;
	if (ImGui::IsItemClicked())
		SetCurrentFolder(&info);

	if (open)
	{
		for (auto& node : info.Children)
			ShowFolderTreeNode(node);

		ImGui::TreePop();
	}

	return CurrentFolderContents.Folder == &info;
}

void AssetBrowserPanel::ApplyFileFilter()
{
	auto* item = CurrentFolderContents.Reset();
	while (item)
	{
		item->Filtered = stristr(item->Name.c_str(), FilterText) == nullptr;
		item = CurrentFolderContents.Next();
	}
}
bool AssetBrowserPanel::CanNavigateBack()
{
	return NavigationHistory.empty() || NavigationHistoryIndex > 0;
}

void AssetBrowserPanel::NavigateBack()
{
	if (NavigationHistoryIndex > 0)
		NavigationHistoryIndex--;
	ApplyCurrentFolder(NavigationHistory[NavigationHistoryIndex]);
}

bool AssetBrowserPanel::CanNavigateForward()
{
	return NavigationHistoryIndex + 1 < NavigationHistory.size();
}

void AssetBrowserPanel::NavigateForward()
{
	if (NavigationHistoryIndex + 1 < NavigationHistory.size())
		NavigationHistoryIndex++;
	ApplyCurrentFolder(NavigationHistory[NavigationHistoryIndex]);
}

void AssetBrowserPanel::LoadSettings(rapidjson::Document& settings)
{
	if (!settings.HasMember("AssetBrowserSettings"))
		return;

	auto settingsValue = settings.FindMember("AssetBrowserSettings");

	bool isIcon = JSONHelper::GetValue<bool>(settingsValue->value, "IconView", false);

	if (isIcon)
		CurrentView = &GridView;
	else
		CurrentView = &ListView;
}

void AssetBrowserPanel::SaveSettings(rapidjson::Document& settings)
{
	rapidjson::Value defaultSettings("AssetBrowserSettings", settings.GetAllocator());
	defaultSettings.SetObject();

	if (!settings.HasMember("AssetBrowserSettings"))
		settings.AddMember("AssetBrowserSettings", defaultSettings, settings.GetAllocator());

	auto settingsValue = settings.FindMember("AssetBrowserSettings");

	bool isIcon = CurrentView == &GridView;
	JSONHelper::SetOrCreateValue(settingsValue->value, "IconView", isIcon, settings);
}

void AssetBrowserPanel::ShowFilePane()
{
	if (!CurrentView)
		return;

	auto* item = CurrentView->Show(CurrentFolderContents);
	if (item)
	{
		AssetItems::AssetItemInfo* assetItem = static_cast<AssetItems::AssetItemInfo*>(item);
		if (assetItem->IsFile())
		{
			AssetItems::FileInfo* file = static_cast<AssetItems::FileInfo*>(assetItem);

			Application::GetInstance().OpenAssetDocument(file->FullPath);
		}
		else
		{
			SetCurrentFolder(static_cast<AssetItems::FolderInfo*>(item));
		}
	}
}

void AssetBrowserPanel::SetCurrentFolder(AssetItems::FolderInfo* folder)
{
	if (CurrentFolderContents.Folder == folder)
		return;

	while (NavigationHistory.size() > NavigationHistoryIndex + 1)
	{
		NavigationHistory.erase(NavigationHistory.begin() + NavigationHistory.size() - 1);
	}

	NavigationHistoryIndex++;
	NavigationHistory.push_back(folder);

	ApplyCurrentFolder(folder);
}

void AssetBrowserPanel::ApplyCurrentFolder(AssetItems::FolderInfo* folder)
{
	AssetItems::FolderInfo* openFolder = folder;
	while (openFolder != nullptr)
	{
		openFolder->ForceOpenNextFrame = true;
		openFolder = openFolder->Parent;
	}

	CurrentFolderContents.Files.clear();

	auto* item = CurrentFolderContents.Reset();
	while (item)
	{
		item->Filtered = false;
		item = CurrentFolderContents.Next();
	}

	AssetItems::AssetFolderPopulator::Populate(CurrentFolderContents, folder);

	ApplyFileFilter();
}

void  AssetBrowserPanel::ShowHeader()
{
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

	if (ImGui::BeginChild("Header", ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() }))
	{
		float size = ImGui::CalcTextSize(ICON_FA_LIST_UL).x + ImGui::GetStyle().FramePadding.x * 2.0f;

		if (ImGui::Button(CurrentView == &ListView ? ICON_FA_BORDER_ALL : ICON_FA_LIST_UL, ImVec2(size, 0)))
		{
			if (CurrentView == &ListView)
				CurrentView = &GridView;
			else
				CurrentView = &ListView;
		}
		ImGui::SameLine(0, spacing);

		ImGui::BeginDisabled(!CanNavigateBack());
		if (ImGui::Button(ICON_FA_ARROW_LEFT))
			NavigateBack();

		ImGui::EndDisabled();

		ImGui::SameLine(0, spacing);
		ImGui::BeginDisabled(!CanNavigateForward());
		if (ImGui::Button(ICON_FA_ARROW_RIGHT))
			NavigateForward();

		ImGui::EndDisabled();

		ImGui::SameLine(0, spacing);
		ImGui::BeginDisabled(!CurrentFolderContents.Folder->Parent);
		if (ImGui::Button(ICON_FA_UPLOAD))
			SetCurrentFolder(CurrentFolderContents.Folder->Parent);

		ImGui::EndDisabled();

		ImGui::SameLine(0, spacing);
		if (ImGui::Button(ICON_FA_ARROW_ROTATE_RIGHT))
			ApplyCurrentFolder(CurrentFolderContents.Folder);

		ImGui::SameLine(0, spacing);
		ImGui::Text("%s Root", ICON_FA_FOLDER_OPEN);
		ImGui::SameLine(0, spacing);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.33f);
		ImGui::BeginDisabled(true);
		ImGui::InputText("###Path", (char*)(CurrentFolderContents.Folder->RelativePath.c_str()), CurrentFolderContents.Folder->RelativePath.size(), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		ImGui::SameLine(0, spacing);
		ImGui::Text("Filter %s", ICON_FA_FILTER);
		ImGui::SameLine(0, spacing);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		if (ImGui::InputText("###Filter", FilterText, 127))
		{
			ApplyFileFilter();
		}
	}
	ImGui::EndChild();
}
