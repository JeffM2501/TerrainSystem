#pragma once

#include "Panel.h"

#include "type_values.h"
#include "PropertyEditor.h"
#include "AssetEditManager.h"
#include "AssetManager.h"
#include "ItemView.h"

namespace AssetItems
{
	class AssetItemInfo : public ViewableItem
	{
	protected:
		bool File = false;

	public:
		AssetItemInfo(bool file) : File(file) {}
		bool IsFile() const { return File; }
	};

	class FileInfo : public AssetItemInfo
	{
	public:
		FileInfo() : AssetItemInfo(true) {}

		AssetSystem::AssetManager::FileSystemPath FullPath;
	};

	class FolderInfo : public AssetItemInfo
	{
	public:
		FolderInfo() : AssetItemInfo(false) {}

		AssetSystem::AssetManager::FileSystemPath FullPath;
		AssetSystem::AssetManager::AssetPath RelativePath;
		FolderInfo* Parent = nullptr;
		std::list<FolderInfo> Children;

		bool ForceOpenNextFrame = false;

		void PopulateChildren();
	};

	class AssetContainer : public ViewableItemContainer
	{
	public:
		ViewableItem* Reset() override;
		size_t Count() override;
		ViewableItem* Next() override;

		FolderInfo* Folder = nullptr;

		std::vector<FileInfo> Files;

		std::vector<FileInfo>::iterator FileItr;
		std::list<FolderInfo>::iterator FolderItr;

		std::vector<FolderInfo*> OpenTreeFolders;

		bool UseFolders = true;
	};

	class AssetFolderPopulator
	{
	public:
		static void Populate(AssetContainer& container, FolderInfo* root);
	};

	class AssetSearchPopulator
	{
	public:
		static void Populate(AssetContainer& container, FolderInfo* root, const char* search);
	};
}

class AssetBrowserPanel : public EditorFramework::Panel
{
public:
	DEFINE_PANEL(AssetBrowserPanel);

	AssetBrowserPanel();

protected:
	void OnShow() override;

	void ShowHeader();
	void ShowFolderTree();
	bool ShowFolderTreeNode(AssetItems::FolderInfo& info);

	void ApplyFileFilter();

	bool CanNavigateBack();
	void NavigateBack();

	bool CanNavigateForward();
	void NavigateForward();

	void SetCurrentFolder(AssetItems::FolderInfo* folder);
	void ApplyCurrentFolder(AssetItems::FolderInfo* folder);

	void RebuildFolderTree();


private:
	ListItemView ListView;
	GridItemView GridView;

	ItemView* CurrentView = nullptr;

	AssetItems::FolderInfo FolderRoot;
	AssetItems::AssetContainer CurrentFolderContents;

	char FilterText[128] = { 0 };
};
