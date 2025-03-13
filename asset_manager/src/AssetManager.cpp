#include "AssetManager.h"
#include "AssetDatabase.h"
#include "type_database.h"
#include "type_io.h"
#include <unordered_map>

namespace AssetSystem
{
	namespace AssetManager
	{
		struct AssetFileRecord
		{
			std::shared_ptr<TypeWraper> AssetPtr;
			std::string Path;
			size_t RefCount = 0;
		};

		static std::unordered_map<Hashes::GUID, AssetFileRecord> OpenedAssets;
		static std::unordered_map<std::string, Hashes::GUID> OpenedAssetsByPath;

		TypeDatabase TypeDB;

		AssetTypes::Asset* FindExistingAsset(const std::string& assetFilePath)
		{
			auto existingAsset = OpenedAssetsByPath.find(assetFilePath);
			if (existingAsset != OpenedAssetsByPath.end())
			{
				auto& info = OpenedAssets[existingAsset->second];
				info.RefCount++;
				return static_cast<AssetTypes::Asset*>(info.AssetPtr.get());
			}
			return nullptr;
		}

		AssetTypes::Asset* FindExistingAsset(const Hashes::GUID& assetGUID)
		{
			auto existingAsset = OpenedAssets.find(assetGUID);
			if (existingAsset != OpenedAssets.end())
			{
				auto& info = existingAsset->second;
				info.RefCount++;

				return static_cast<AssetTypes::Asset*>(info.AssetPtr.get());
			}

			return nullptr;
		}

		void StoreAsset(const std::string& assetFilePath, std::shared_ptr<TypeWraper> assetData)
		{
			AssetTypes::Asset* asset = static_cast<AssetTypes::Asset*>(assetData.get());

			OpenedAssets[asset->GetMeta().GetGUID()] = AssetFileRecord{ assetData, assetFilePath, 1 };
			OpenedAssetsByPath[assetFilePath] = asset->GetMeta().GetGUID();
		}

		void CloseAsset(AssetTypes::Asset* asset)
		{
			auto guid = asset->GetMeta().GetGUID();

			auto itr = OpenedAssets.find(guid);
			if (itr != OpenedAssets.end())
				return;

			itr->second.RefCount--;
			if (itr->second.RefCount == 0)
			{
				OpenedAssetsByPath.erase(OpenedAssetsByPath.find(itr->second.Path));
				OpenedAssets.erase(itr);
			}
		}

	}
}
