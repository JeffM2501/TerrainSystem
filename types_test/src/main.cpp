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
    Types::RegisterTypes(AssetSystem::AssetManager::TypeDB);

    auto& db = AssetSystem::AssetManager::TypeDB;

    TestDerivedType test3 = db.CreateTypeValue<AssetTypes::TestDerivedType>();
    auto strDefault = test3.GetDerivedString();// test3.SetDerivedString("I was also set");


    auto matAsset = db.CreateTypeValue<AssetTypes::TerrainMaterialAsset>();

    auto path = matAsset.GetPath();

    return 0;
}