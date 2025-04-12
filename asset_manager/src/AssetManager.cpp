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

		static std::unordered_map<std::string, AssetFileRecord> OpenedAssets;
		std::unordered_map<size_t, std::unique_ptr<TypeWraper>> TempAssets;

		AssetTypes::Asset* FindExistingAsset(const std::string& assetFilePath)
		{
			auto existingAsset = OpenedAssets.find(assetFilePath);
			if (existingAsset != OpenedAssets.end())
			{
				auto& info = existingAsset->second;
				info.RefCount++;
				return static_cast<AssetTypes::Asset*>(info.AssetPtr.get());
			}
			return nullptr;
		}

		void StoreAsset(const std::string& assetFilePath, std::unique_ptr<TypeWraper> assetData)
		{
			OpenedAssets[assetFilePath] = AssetFileRecord{ std::move(assetData), assetFilePath, 1 };
		}

		void CloseAsset(AssetTypes::Asset* asset)
		{
			auto& path = asset->GetPath();

			auto itr = OpenedAssets.find(path);
			if (itr != OpenedAssets.end())
				return;

			itr->second.RefCount--;
			if (itr->second.RefCount == 0)
			{
				OpenedAssets.erase(itr);
			}
		}
	}
}
