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
		extern std::unordered_map<size_t, std::unique_ptr<TypeWraper>> TempAssets;

		AssetTypes::Asset* FindExistingAsset(const std::string& assetFilePath);

		void CloseAsset(AssetTypes::Asset* asset);

		void StoreAsset(const std::string& assetFilePath, std::unique_ptr<TypeWraper> assetData);

		template <class T>
		T* OpenAsset(const std::string& assetFilePath)
		{
			auto existing = FindExistingAsset(assetFilePath);

			if (existing)
			{
				if (existing->TypeName != T::TypeName)
					return nullptr;
				return static_cast<T*>(existing);
			}
			// todo make the path relative to asset root not working dir
			std::unique_ptr<T> asset = T::ReadAs<T>(assetFilePath, true);

			if (!asset->IsValid())
			{
				return nullptr;
			}

			// set the current path in the asset
			// TODO, see if the path is different?
			asset->SetPath(assetFilePath);

			T* ptr = asset.get();
			StoreAsset(assetFilePath, std::move(asset));

			return ptr;
		}

		template <class T>
		T* CreateTempAsset()
		{
			std::unique_ptr<T> asset = std::make_unique<T>();
			T* ptr = asset.get();

			TempAssets[reinterpret_cast<size_t>(ptr)] = std::move(asset);

			return ptr;
		}

		template <class T>
		T* CreateAsset(const std::string& assetFilePath)
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
	}
}