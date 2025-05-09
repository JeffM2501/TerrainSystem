#pragma once

#include "CRC64.h"
#include "type_values.h"
#include "AssetReference.h"
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
		size_t AssetId;
		FieldPath	ValuePath;
	};

	class FieldEditAction : public EditAction
	{
	public:
		ValueChangedRecord::Ptr ValueRecord;
	};
	std::vector<std::unique_ptr<EditAction>> Actions;

	bool AllowMerge = true;

	AssetEditEventRecord(std::string_view name, size_t mergeID);

	std::string Name;

	template<class T>
	T* PushAction()
	{
		Actions.emplace_back(std::make_unique<T>());
		return static_cast<T*>(Actions.back().get());
	}

	bool IsEmpty() const { return Actions.empty(); }

	void CheckMergeability();

	FieldPath CommonAncestorPath;
	std::string FieldName;
};

class AssetEditManager
{
protected:
	std::unordered_map<uint64_t, Types::TypeValue*> OpenAssets;
	std::unordered_map<uint64_t, AssetTypes::Asset*> OpenAssetsRefs;

	Tokens::TokenSource Token;

	void RegisterEditCallbacks(Types::TypeValue* assetData);

	std::vector<std::unique_ptr<AssetEditEventRecord>> EditEvents;
	size_t CurrentEditEventIndex = 0;

	std::unique_ptr<AssetEditEventRecord> CurrentEditEvent;

	static AssetEditManager* CurrentEditManager;

public:

	AssetEditManager() = default;
	AssetEditManager(const AssetEditManager&) = delete;
	AssetEditManager& operator = (const AssetEditManager&) = delete;

	~AssetEditManager() { CloseAll(); }


	void Activate() { CurrentEditManager = this; }
	void Deactivate() { CurrentEditManager = this; }

	static AssetEditManager* GetCurrent() { return CurrentEditManager; }

	template <class T>
	T* OpenAsset(const std::string& assetFilePath)
	{
		T* asset = AssetSystem::AssetManager::OpenAsset<T>(assetFilePath);
		if (asset)
		{
			uint64_t hash = Hashes::CRC64Str(assetFilePath);
			if (OpenAssets.find(hash) != OpenAssets.end())
				return asset;

			asset->ValuePtr->ID = hash;
			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			OpenAssetsRefs.insert_or_assign(hash, asset);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	template <class T>
	T* CreateTempAsset()
	{
		T* asset = AssetSystem::AssetManager::CreateTempAsset<T>();
		if (asset)
		{
			uint64_t hash = uint64_t(asset);
			asset->ValuePtr->ID = hash;
			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			OpenAssetsRefs.insert_or_assign(hash, asset);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	template <class T>
	T* CreateAsset(const std::string& assetFilePath)
	{
		T* asset = AssetSystem::AssetManager::CreateAsset<T>(assetFilePath);
		if (asset)
		{
			uint64_t hash = Hashes::CRC64Str(assetFilePath);
			if (OpenAssets.find(hash) != OpenAssets.end())
				return asset;

			asset->ValuePtr->ID = hash;
			OpenAssets.insert_or_assign(hash, asset->ValuePtr);
			OpenAssetsRefs.insert_or_assign(hash, asset);
			RegisterEditCallbacks(asset->ValuePtr);
		}

		return asset;
	}

	void CloseAsset(AssetTypes::Asset* asset)
	{
		AssetSystem::AssetManager::CloseAsset(asset);

		uint64_t hash = asset->ValuePtr->ID;
		auto itr = OpenAssets.find(hash);
		if (itr != OpenAssets.end())
			OpenAssets.erase(itr);

		auto refItr = OpenAssetsRefs.find(hash);
		if (refItr != OpenAssetsRefs.end())
			OpenAssetsRefs.erase(refItr);
	}

	void CloseAll()
	{
		for (auto& [hash, assetRef] : OpenAssetsRefs)
		{
			AssetSystem::AssetManager::CloseAsset(assetRef);
		}

		OpenAssetsRefs.clear();
		OpenAssets.clear();
	}

	bool SaveAssetAs(AssetTypes::Asset* asset, const AssetSystem::AssetManager::AssetPath& assetPath)
	{
		return AssetSystem::AssetManager::SaveAssetAs(asset, assetPath);
	}

	bool SaveAsset(AssetTypes::Asset* asset)
	{
		return AssetSystem::AssetManager::SaveAsset(asset);
	}

	void BeginEvent(std::string_view eventName, bool allowMerge = true);
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

template<class T>
class AssetReferenceResolver
{
private:
	AssetEditManager& Manager;
public:
    T* AssetValue = nullptr;
	Tokens::TokenSource	Token;

	AssetTypes::AssetReference Reference;

	Events::EventSource<AssetReferenceResolver<T>> ReferenceChanged;
    Events::EventSource<AssetReferenceResolver<T>> ReferenceDataChanged;

	Events::EventSource<AssetReferenceResolver<T>> AssetReferenceClosed;
    Events::EventSource<AssetReferenceResolver<T>> AssetReferenceOpened;

	AssetReferenceResolver(AssetEditManager& manager, Types::TypeValue* value)
		: Manager(manager)
		, Reference(value)
	{
		value->OnValueChanged.Add([this](ValueChangedEvent evt) 
			{
				Resolve();
			}
		, Token.GetToken());
	}

	AssetReferenceResolver(const AssetReferenceResolver&) = delete;
	AssetReferenceResolver& operator = (const AssetReferenceResolver&) = delete;

	virtual ~AssetReferenceResolver()
	{
		Close();
	}

	bool IsValid() const { return AssetValue != nullptr; }

	bool Resolve()
    {
		// we have an asset
		if (AssetValue)
		{
			// it's the same asset, so do nothing
			if (AssetValue->GetPath() == Reference.GetPath())
				return false;
		}
		
		Close();
		AssetValue = Manager.OpenAsset<T>(Reference.GetPath());

		if (AssetValue)
		{
            AssetValue->Value->OnValueChanged.Add([this](const ValueChangedEvent& evt) {ReferenceDataChanged.Invoke(*this); }, Token.GetToken());
            AssetReferenceOpened.Invoke(*this);
            ReferenceDataChanged.Invoke(*this);

			return true;
		}

		return false;
	}

    void Close()
    {
        if (AssetValue)
        {
            // remove all event handlers from the asset we are going to close
            AssetValue->Value->OnValueChanged.Remove(Token.GetToken());
            AssetReferenceClosed.Invoke(*this);

            // we are done referencing this asset
            Manager.CloseAsset(AssetValue);

            AssetValue = nullptr;
        }
    }
};