#pragma once

#include "CRC64.h"
#include "type_values.h"
#include "AssetManager.h"
#include "LifetimeToken.h"
#include "Events.h"

#include <unordered_map>
#include <string>
#include <vector>

class AssetEditManager;

class AssetEditEventRecord
{
private:
	friend class AssetEditManager;

	class EditAction
	{
	public:
	};
	std::vector<EditAction> Actions;

	size_t MergeID = 0;

	AssetEditEventRecord(std::string_view name, size_t mergeID);

public:
	std::string Name;

	size_t GetMergeID() const { return MergeID; }

	void PushAction(const EditAction& action);
};

class AssetEditManager
{
protected:
	std::unordered_map < uint64_t, Types::TypeValue*> OpenAssets;

	Tokens::TokenSource Token;

	void RegisterEditCallbacks(Types::TypeValue* assetData);

	std::vector<AssetEditEventRecord> EditEvents;
	size_t CurrentEditEvent = 0;
public:
	template <class T>
	T* OpenAsset(const std::string& assetFilePath)
	{
		T* asset = AssetManager::OpenAsset<T>(assetFilePath);
		if (asset)
		{
			uint64_t hash = Hashes::CRC64Str(assetFilePath);
			if (OpenAssets.find(hash) != OpenAssets.end())
				return asset;

			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	template <class T>
	T* CreateTempAsset()
	{
		T* asset = AssetManager::CreateTempAsset<T>(assetFilePath);
		if (asset)
		{
			uint64_t hash = uint64_t(asset);
			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	template <class T>
	T* CreateAsset(const std::string& assetFilePath)
	{
		T* asset = AssetManager::CreateAsset<T>(assetFilePath);
		if (asset)
		{
			uint64_t hash = Hashes::CRC64Str(assetFilePath);
			if (OpenAssets.find(hash) != OpenAssets.end())
				return asset;

			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	void BeginEvent(std::string_view eventName, size_t mergeID = 0);
	void FinalizeEvent();

	bool CanUndo() const;
	bool CanRedo() const;

	void Undo();
	void Redo();

	const std::vector<AssetEditEventRecord>& GetEditEvents() const { return EditEvents; }
	size_t GetCurrentEventIndex() const { return CurrentEditEvent; }

	class AssetDirtyEvent
	{
	public:
		uint64_t	AssetID;
	};

	Events::EventSource<AssetDirtyEvent> OnAssetDirty;
};