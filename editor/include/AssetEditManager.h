#pragma once

#include "CRC64.h"
#include "type_values.h"
#include "AssetManager.h"
#include "LifetimeToken.h"
#include "Events.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

class AssetEditManager;

class AssetEditEventRecord
{
private:
	friend class AssetEditManager;

public:
	class EditAction
	{
	public:
	};

	class PrimitiveFieldEditAction : public EditAction
	{
	public:
		size_t AssetId;
		FieldPath	ValuePath;

		PrimitiveValueChangedEvent PreviousValue;
		PrimitiveValueChangedEvent CurrentValue;
	};
	std::vector<std::unique_ptr<EditAction>> Actions;

	size_t MergeID = 0;

	AssetEditEventRecord(std::string_view name, size_t mergeID);

	std::string Name;

	size_t GetMergeID() const { return MergeID; }

	template<class T>
	T* PushAction()
	{
		Actions.emplace_back(std::make_unique<T>());
		return static_cast<T*>(Actions.back().get());
	}

    bool IsEmpty() const { return Actions.empty(); }
};

class AssetEditManager
{
protected:
	std::unordered_map < uint64_t, Types::TypeValue*> OpenAssets;

	Tokens::TokenSource Token;

	void RegisterEditCallbacks(Types::TypeValue* assetData);

	std::vector<std::unique_ptr<AssetEditEventRecord>> EditEvents;
	size_t CurrentEditEventIndex = 0;

	std::unique_ptr<AssetEditEventRecord> CurrentEditEvent;

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

			asset->ValuePtr->ID = hash;
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
			asset->ValuePtr->ID = hash;
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

			asset->ValuePtr->ID = hash;
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

	const std::vector<std::unique_ptr<AssetEditEventRecord>>& GetEditEvents() const { return EditEvents; }
	size_t GetCurrentEventIndex() const { return CurrentEditEventIndex; }

	class AssetDirtyEvent
	{
	public:
		uint64_t	AssetID;
	};

	Events::EventSource<AssetDirtyEvent> OnAssetDirty;
};