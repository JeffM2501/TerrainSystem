#include "AssetDatabase.h"
#include "types/asset.h"
#include <unordered_map>

namespace AssetSystem
{
    namespace AssetTypeDatabase
    {
        std::vector<AssetTypeRecord> AssetTypeDB;
        std::unordered_map<uint64_t, size_t> AssetIDTypeDB;

        const std::vector<AssetTypeRecord>& GetAssetTypes()
        {

            return AssetTypeDB;
        }

        const AssetTypeRecord* GetAssetTypeInfo(uint64_t typeId)
        {
            auto itr = AssetIDTypeDB.find(typeId);
            if (itr == AssetIDTypeDB.end())
                return nullptr;

            return &AssetTypeDB[itr->second];
        }

        void Init()
        {
            auto& db = Types::TypeDatabase::Get();

            uint64_t assetTypeId = db.GetTypeId(AssetTypes::Asset::TypeName);

            db.ItterateTypes([&db, assetTypeId](uint64_t id, const Types::TypeInfo* typeInfo)
                {
                    if (id == assetTypeId || !db.IsBaseClassOf(id, assetTypeId))
                        return;

                    AssetTypeRecord record;
                    record.TypeID = id;
                    record.Type = typeInfo;

                    if (typeInfo->HasAttribute<AssetTypes::AssetIconAttribute>())
                    {
                        record.Icon = typeInfo->GetAttribute<AssetTypes::AssetIconAttribute>()->Icon;
                    }

                    if (typeInfo->HasAttribute<AssetTypes::FileExtensionAttribute>())
                    {
                        record.Extension = typeInfo->GetAttribute<AssetTypes::FileExtensionAttribute>()->Extension;
                    }

                    AssetTypeDB.push_back(record);
                    AssetIDTypeDB.insert_or_assign(id, AssetIDTypeDB.size() - 1);

                });
        }
    }
}