#pragma once


#include <string>
#include <memory>
#include <functional>
#include <cstdint>

#include "GUID.h"
#include "CRC64.h"

#include "types/asset.h"

namespace AssetSystem
{
	namespace AssetManager
	{
		extern std::unordered_map<size_t, std::shared_ptr<TypeWraper>> TempAssets;

		void SetAssetRoot(const std::string& path);

		using AssetPath = std::string;
		using FileSystemPath = std::string;

		bool IsPathInRoot(const FileSystemPath& path);
		AssetPath ToAssetPath(const FileSystemPath& path);
		FileSystemPath ToFileSystemPath(const AssetPath& path);


		AssetTypes::Asset* FindExistingAsset(const std::string& assetFilePath);

		void CloseAsset(AssetTypes::Asset* asset);

		void StoreAsset(const std::string& assetFilePath, std::unique_ptr<TypeWraper> assetData);

		template <class T>
		T* OpenAsset(const AssetPath& assetFilePath)
		{
			auto existing = FindExistingAsset(assetFilePath);

			if (existing)
			{
				if (existing->TypeName != T::TypeName)
					return nullptr;
				return static_cast<T*>(existing);
			}
			// todo make the path relative to asset root not working dir

			FileSystemPath filePath = ToFileSystemPath(assetFilePath);
			std::unique_ptr<T> asset = T::ReadAs<T>(filePath, true);

			if (!asset->IsValid())
			{
				return nullptr;
			}

			// set the current path in the asset
			asset->SetPath(assetFilePath);

			T* ptr = asset.get();
			StoreAsset(assetFilePath, std::move(asset));

			return ptr;
		}

		template <class T>
		T* CreateTempAsset()
		{
			std::shared_ptr<T> asset = std::make_unique<T>();
			T* ptr = asset.get();

			TempAssets[reinterpret_cast<size_t>(ptr)] = asset;

			return ptr;
		}

		template <class T>
		T* CreateAsset(const AssetPath& assetFilePath)
		{
			auto existing = FindExistingAsset(assetFilePath);

			if (existing)
			{
				if (existing->TypeName != T::TypeName)
					return nullptr;
				return existing;
			}

			std::unique_ptr<T> asset = std::unique_ptr<T>();
			T* ptr = asset.get();

			if (!asset->IsValid())
			{
				return nullptr;
			}

			// set the current path in the asset
			asset->SetPath(assetFilePath);

			StoreAsset(assetFilePath, std::move(asset));

			return ptr;
		}

		bool SaveAssetAs(AssetTypes::Asset* asset, const AssetPath& assetPath);

		bool SaveAsset(AssetTypes::Asset* asset);
	}
}