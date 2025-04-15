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
			AssetPath Path;
			size_t RefCount = 0;
		};

		static std::unordered_map<std::string, AssetFileRecord> OpenedAssets;
		std::unordered_map<size_t, std::shared_ptr<TypeWraper>> TempAssets;
		Events::EventSource<std::string> AssetRootChanged;

		std::string AssetRootPath;

		void SanitizePath(std::string& path)
		{
			size_t pos = 0;
			std::string from("\\");
			std::string to("/");

			while ((pos = path.find(from, pos)) != std::string::npos)
			{
				path.replace(pos, from.length(), to);
				pos += to.length(); // Handles cases where 'to' is a substring of 'from' or they overlap
			}
		}

		void SetAssetRoot(const std::string& path)
		{
			AssetRootPath = path;
			SanitizePath(AssetRootPath);
			if (!AssetRootPath.empty() && *AssetRootPath.rbegin() != '/')
				AssetRootPath += "/";

			AssetRootChanged.Invoke(AssetRootPath);
		}

		bool IsPathInRoot(const FileSystemPath& path)
		{
			FileSystemPath cleanPath = path;
			SanitizePath(cleanPath);

			auto folder = cleanPath.substr(0, AssetRootPath.size());
			return folder == AssetRootPath;
		}

		AssetPath ToAssetPath(const FileSystemPath& path)
		{
			if (!IsPathInRoot(path))
				return AssetPath();

			AssetPath assetPath = path.substr(AssetRootPath.size());
			SanitizePath(assetPath);
			return assetPath;
		}

		FileSystemPath ToFileSystemPath(const AssetPath& path)
		{
			return AssetRootPath + path;
		}

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
			if (itr == OpenedAssets.end())
				return;

			itr->second.RefCount--;
			if (itr->second.RefCount == 0)
			{
				OpenedAssets.erase(itr);
			}
		}

		bool SaveAsset(AssetTypes::Asset* asset)
		{
			if (TempAssets.find(reinterpret_cast<size_t>(asset)) != TempAssets.end())
			{
				return false;
			}

			AssetPath path = asset->GetPath();
			FileSystemPath filePath = ToFileSystemPath(path);

			return asset->Write(filePath);
		}

		bool SaveAssetAs(AssetTypes::Asset* asset, const AssetPath& assetPath)
		{
			AssetPath oldPath = asset->GetPath();

			if (!oldPath.empty())
			{
				auto record = OpenedAssets[oldPath];
				OpenedAssets.erase(OpenedAssets.find(oldPath));
				record.Path = assetPath;
				OpenedAssets[assetPath] = record;
			}

			// make it real
			auto itr = TempAssets.find(reinterpret_cast<size_t>(asset));
			if (itr != TempAssets.end())
			{
				OpenedAssets[assetPath].AssetPtr = itr->second;
				TempAssets.erase(itr);
			}
			asset->SetPath(assetPath);

			return SaveAsset(asset);
		}
	}
}
