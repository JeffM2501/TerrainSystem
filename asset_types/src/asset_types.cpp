#include "asset_types.h"

#include "types/test_type.h"
#include "types/asset.h"
#include "types/terrain.h"

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
        AssetTypes::Asset::Register(typeDB);

		AssetTypes::TerrainPosition::Register(typeDB);
		AssetTypes::TerrainMaterial::Register(typeDB);
        AssetTypes::TerrainMaterialAsset::Register(typeDB);
		AssetTypes::TerrainInfo::Register(typeDB);
		AssetTypes::TerrainSplatmap::Register(typeDB);
        AssetTypes::TerrainTile::Register(typeDB);
        AssetTypes::TerrainAsset::Register(typeDB);
    }
}
