#include "raylib.h"

#include "type_database.h"
#include "AssetManager.h"

#include "asset_types.h"
#include "types/test_type.h"
#include "types/asset.h"
#include "types/terrain.h"

using namespace Types;
using namespace TypeIO;
using namespace AssetTypes;

int main(int argc, char* argv[])
{
	AssetTypes::RegisterTypes();

	TestDerivedType test3 = TypeDatabase::Get().CreateTypeValue<AssetTypes::TestDerivedType>();
	auto strDefault = test3.GetDerivedString();// test3.SetDerivedString("I was also set");

	auto matAsset = TypeDatabase::Get().CreateTypeValue<AssetTypes::TerrainMaterialAsset>();

	auto path = matAsset.GetPath();
	return 0;
}
