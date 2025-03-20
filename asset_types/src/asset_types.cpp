#include "asset_types.h"

#include "types/test_type.h"
#include "types/asset.h"
#include "types/terrain.h"

namespace Types
{
	void RegisterTypes()
	{
		AssetTypes::TestType::Register();
		AssetTypes::TestTypeWithEmbed::Register();
		AssetTypes::TestDerivedType::Register();
		AssetTypes::TestContainerType::Register();
		AssetTypes::TestTypeWithPointer::Register();
		AssetTypes::TestTypeWithTypeList::Register();
		AssetTypes::TestTypeWithPtrTypeList::Register();

		AssetTypes::ResourceReference::Register();
		AssetTypes::AssetReference::Register();
		AssetTypes::Asset::Register();

		AssetTypes::TerrainPosition::Register();
		AssetTypes::TerrainMaterial::Register();
		AssetTypes::TerrainMaterialAsset::Register();
		AssetTypes::TerrainInfo::Register();
		AssetTypes::TerrainSplatmap::Register();
		AssetTypes::TerrainTile::Register();
		AssetTypes::TerrainAsset::Register();
	}
}
