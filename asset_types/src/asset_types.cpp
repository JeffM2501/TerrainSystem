#include "asset_types.h"

#include "types/test_type.h"
#include "types/asset.h"

namespace Types
{
    void RegisterTypes(TypeDatabase& typeDB)
    {
        AssetTypes::TestType::Register(typeDB);
        AssetTypes::TestTypeWithEmbed::Register(typeDB);
        AssetTypes::TestDerivedType::Register(typeDB);
        AssetTypes::TestContainerType::Register(typeDB);
        AssetTypes::TestTypeWithPointer::Register(typeDB);
        AssetTypes::TestTypeWithTypeList::Register(typeDB);
        AssetTypes::TestTypeWithPtrTypeList::Register(typeDB);


        AssetTypes::ResourceReference::Register(typeDB);
        AssetTypes::AssetReference::Register(typeDB);
        AssetTypes::AssetMetaInfo::Register(typeDB);
        AssetTypes::Asset::Register(typeDB);
    }
}
