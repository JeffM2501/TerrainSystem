#pragma once

#include "type_database.h"

#include <string>
#include <vector>

namespace AssetSystem
{
	namespace AssetTypeDatabase
	{
		struct AssetTypeRecord
		{
			uint64_t TypeID = 0;
			const Types::TypeInfo* Type = nullptr;
			std::string Extension;
			std::string Icon;
		};

		const std::vector<AssetTypeRecord>& GetAssetTypes();
		const AssetTypeRecord* GetAssetTypeInfo(uint64_t typeId);

		template<class T>
		const AssetTypeRecord* GetAssetTypeInfo()
		{
			return GetAssetTypeInfo(Types::TypeDatabase::GetTypeId(T::TypeName));
		}
		void Init();
	}
}