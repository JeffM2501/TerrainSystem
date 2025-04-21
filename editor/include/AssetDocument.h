#pragma once

#include "Document.h"

#include "types/asset.h"
#include "Application.h"
#include "PropertiesPanel.h"
#include "AssetDatabase.h"
#include "AssetEditManager.h"
#include "AssetManager.h"

#include "raylib.h"
#include <vector>
#include <string>

template<class T>
class AssetDocument : public EditorFramework::ViewportDocument
{
public:
	std::string_view GetDocumentName()
	{
		if (RootAsset && !RootAsset->GetPath().empty())
			return GetFileNameWithoutExt(RootAsset->GetPath().c_str());

		static std::string unknownName;
		unknownName = "Untitled ";
		unknownName += T::TypeName;

		return unknownName;
	}

	void CreateAsset() override
	{
		RootAsset = EditManager.CreateTempAsset<T>();
		RegisterEditHandler();
		OnAssetCreate();
		OnAssetOpen();
	}

	void OpenAsset(const std::string& assetPath) override
	{
		RootAsset = EditManager.OpenAsset<T>(AssetSystem::AssetManager::ToAssetPath(assetPath));
		OnActivated();
		RegisterEditHandler();
		OnAssetOpen();
	}

	void SaveAsset() override
	{
		if (!RootAsset)
			return;
		EditManager.SaveAsset(RootAsset);
		SetClean();
		OnAssetSave();
	}

	void SaveAsAsset(const std::string& assetPath) override
	{
		if (!RootAsset)
			return;
		EditManager.SaveAssetAs(RootAsset, AssetSystem::AssetManager::ToAssetPath(assetPath));
		SetClean();
		OnAssetSave();
	}

	void OnActivated() override
	{
		auto* properties = GetApp()->GetPanel<PropertiesPanel>();
		if (properties && RootAsset)
			properties->SetObject(RootAsset->ValuePtr);

		EditManager.Activate();
	}

	void OnDeactivated() override
	{
		auto* properties = GetApp()->GetPanel<PropertiesPanel>();
		if (properties)
			properties->SetObject(nullptr);

		EditManager.Deactivate();
	}

	bool UsesAsset() override { return true; }

	std::string_view GetAssetPath() const
	{
		if (RootAsset)
			return RootAsset->GetPath();

		return "";
	}

	virtual ~AssetDocument() {}

protected:
	T* RootAsset = nullptr;

	Tokens::TokenSource Token;

	AssetEditManager EditManager;

	virtual void OnAssetOpen() {};
	virtual void OnAssetCreate() {};
	virtual void OnAssetSave() {};
	virtual void OnAssetDirty() {};

	void RegisterEditHandler()
	{
		RootAsset->ValuePtr->OnValueChanged.Add([this](const Types::ValueChangedEvent& event)
			{
				SetDirty();
				OnAssetDirty();
			}, Token.GetToken());
	}
};