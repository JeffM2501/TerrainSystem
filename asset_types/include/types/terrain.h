#pragma once

#include "type_wrapper.h"

#include "types/asset.h"
#include "raylib.h"
#include "raymath.h"
#include "extras/IconsFontAwesome6.h"

#include <memory>

using namespace Types;

namespace AssetTypes
{
	class TerrainPosition : public TypeWraper
	{
	public:
		DEFINE_TYPE(TerrainPosition);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName);
			type->AddPrimitiveField<int64_t>("X", 0);
			type->AddPrimitiveField<int64_t>("Y", 1);
		}

		const int64_t& GetX() const { return ValuePtr->GetFieldPrimitiveValue<int64_t>(0); }
		void SetX(const int64_t& value) { ValuePtr->SetFieldPrimitiveValue<int64_t>(0, value); }
		void ResetX() { ValuePtr->ResetFieldToDefault(0); }

		const int64_t& GetY() const { return ValuePtr->GetFieldPrimitiveValue<int64_t>(1); }
		void SetY(const int64_t& value) { ValuePtr->SetFieldPrimitiveValue<int64_t>(1, value); }
		void ResetY() { Value->ResetFieldToDefault(1); }
	};

	class TerrainMaterialTypeEnum
	{
	public:
		DEFINE_ENUM(TerrainMaterialTypeEnum);

		enum class Values
		{
			DiffuseOnly = 0,
			Tintable = 1,
			DiffuseAndNomal = 2
		};

		static void Register()
		{
			auto* enumType = TypeDatabase::Get().CreateEnumeration(TypeName);
			enumType->Values.insert_or_assign(0, "DiffuseOnly");
			enumType->Values.insert_or_assign(1, "Tintable");
			enumType->Values.insert_or_assign(2, "DiffuseAndNomal");
		}
	};

	class TerrainMaterial : public TypeWraper
	{
	public:
		DEFINE_TYPE(TerrainMaterial);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName);

			type->AddPrimitiveField<std::string>("Name", "");
			auto* diffuseTexture = type->AddTypeField("DiffuseTexture", ResourceReference::TypeName);
			diffuseTexture->AddAttribute<FileExtensionAttribute>("png");
			diffuseTexture->AddAttribute<AttributeTypes::CustomEditorAttribute>(TextureReferenceEditor);

			type->AddPrimitiveField<Color>("DiffuseColor", Color{ 255,255,255,255 });
			auto* normalMap = type->AddTypeField("NormalMap", ResourceReference::TypeName);
			normalMap->AddAttribute<FileExtensionAttribute>("png");
			normalMap->AddAttribute<AttributeTypes::CustomEditorAttribute>(TextureReferenceEditor);

			type->AddEnumerationField("MaterialType", TerrainMaterialTypeEnum::TypeName, int(TerrainMaterialTypeEnum::Values::DiffuseOnly));
		}

		const std::string& GetName() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
		void SetName(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
		void ResetName() { Value->ResetFieldToDefault(0); }

		ResourceReference GetDiffuseTexture() const { return ResourceReference(ValuePtr->GetTypeFieldValue(1)); }

		const Color& GetDiffuseColor() const { return ValuePtr->GetFieldPrimitiveValue<Color>(2); }
		void SetDiffuseColor(const Color& value) { ValuePtr->SetFieldPrimitiveValue<Color>(2, value); }
		void ResetDiffuseColor() { Value->ResetFieldToDefault(2); }

		ResourceReference GetNormalMap() const { return ResourceReference(ValuePtr->GetTypeFieldValue(3)); }

		TerrainMaterialTypeEnum::Values GetMaterialType() const { return ValuePtr->GetFieldEnumerationValue<TerrainMaterialTypeEnum::Values>(4); }
		void SetMaterialType(const TerrainMaterialTypeEnum::Values& value) { ValuePtr->SetFieldEnumerationValue(4, value); }
		void ResetMaterialType() { Value->ResetFieldToDefault(4); }
	};

	class TerrainMaterialAsset : public Asset
	{
	public:
		DEFINE_DERIVED_TYPE(TerrainMaterialAsset, Asset);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName, Asset::TypeName);
			type->AddAttribute<FileExtensionAttribute>("tmat");
			type->AddAttribute<AttributeTypes::DisplayNameAttribute>("Terrain Material");
			type->AddAttribute<AssetIconAttribute>(ICON_FA_IMAGE);

			type->AddTypeField("Material", TerrainMaterial::TypeName);
			type->AddPrimitiveField<std::string>("Category", "");
		}

		TerrainMaterial GetMaterial() const { return TerrainMaterial(ValuePtr->GetTypeFieldValue(1)); }

		const std::string& GetCategory() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(2); }
		void SetCategory(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(2, value); }
		void ResetCategory() { Value->ResetFieldToDefault(2); }
	};

	class TerrainInfo : public TypeWraper
	{
	public:
		DEFINE_TYPE(TerrainInfo);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName);
			type->AddPrimitiveField<uint8_t>("GridSize", 128);
			type->AddPrimitiveField<float>("TileSize", 128);
			type->AddPrimitiveField<float>("MinZ", -50);
			type->AddPrimitiveField<float>("MaxZ", 100);
		}

		const uint8_t& GetGridSize() const { return ValuePtr->GetFieldPrimitiveValue<uint8_t>(0); }
		void SetGridSize(const uint8_t& value) { ValuePtr->SetFieldPrimitiveValue<uint8_t>(0, value); }
		void ResetGridSize() { ValuePtr->ResetFieldToDefault(0); }

		const float& GetTileSize() const { return ValuePtr->GetFieldPrimitiveValue<float>(1); }
		void SetTileSize(const float& value) { ValuePtr->SetFieldPrimitiveValue<float>(1, value); }
		void ResetTileSize() { Value->ResetFieldToDefault(1); }

		const float& GetMinZ() const { return ValuePtr->GetFieldPrimitiveValue<float>(2); }
		void SetTMinZ(const float& value) { ValuePtr->SetFieldPrimitiveValue<float>(2, value); }
		void ResetMinZ() { Value->ResetFieldToDefault(2); }

		const float& GetMaxZ() const { return ValuePtr->GetFieldPrimitiveValue<float>(3); }
		void SetMaxZ(const float& value) { ValuePtr->SetFieldPrimitiveValue<float>(3, value); }
		void ResetMaxZ() { Value->ResetFieldToDefault(GetParentFieldCount() + 3); }
	};

	class TerrainSplatmap : public TypeWraper
	{
	public:
		DEFINE_TYPE(TerrainSplatmap);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName);

			type->AddPrimitiveField<uint16_t>("Material", 128);
			type->AddPrimitiveListField("Values", Types::PrimitiveType::UInt8);
		}

		const uint16_t& GetMaterial() const { return ValuePtr->GetFieldPrimitiveValue<uint16_t>(0); }
		void SetMaterial(const uint16_t& value) { ValuePtr->SetFieldPrimitiveValue<uint16_t>(0, value); }
		void ResetMaterial() { ValuePtr->ResetFieldToDefault(0); }

		PrimitiveListFieldValue<uint8_t>& GetValues() const { return ValuePtr->GetPrimitiveListFieldValue<uint8_t>(1); }

	};

	class TerrainTile : public TypeWraper
	{
	public:
		DEFINE_TYPE(TerrainTile);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName);
			type->AddTypeField("Origin", TerrainPosition::TypeName);
			type->AddPrimitiveListField("Heightmap", Types::PrimitiveType::Float32);
			type->AddTypeListField("Layers", TerrainSplatmap::TypeName);
		}

		TerrainPosition GetOrigin() const { return TerrainPosition(ValuePtr->GetTypeFieldValue(0)); }
		PrimitiveListFieldValue<float>& GetHeightmap() const { return ValuePtr->GetPrimitiveListFieldValue<float>(1); }
		TypeListWrapper<TerrainSplatmap> GetLayers() const { return TypeListWrapper<TerrainSplatmap>(ValuePtr->GetTypeListFieldValue(2)); }
	};

	class TerrainAsset : public Asset
	{
	public:
		DEFINE_DERIVED_TYPE(TerrainAsset, Asset);

		static void Register()
		{
			auto* type = TypeDatabase::Get().CreateType(TypeName, Asset::TypeName);
			type->AddAttribute<FileExtensionAttribute>("terrain");
			type->AddAttribute<AttributeTypes::DisplayNameAttribute>("Terrain");
			type->AddAttribute<AssetIconAttribute>(ICON_FA_MOUNTAIN);

			type->AddTypeField("Info", TerrainInfo::TypeName);
			type->AddTypeListField("Materials", TerrainMaterial::TypeName);
			type->AddTypeListField("Tiles", TerrainTile::TypeName);
		}

		TerrainInfo GetInfo() const { return TerrainInfo(ValuePtr->GetTypeFieldValue(1)); }
		TypeListWrapper<TerrainMaterial> GetMaterials() const { return TypeListWrapper<TerrainMaterial>(ValuePtr->GetTypeListFieldValue(2)); }
		TypeListWrapper<TerrainTile> GetTiles() const { return TypeListWrapper<TerrainTile>(ValuePtr->GetTypeListFieldValue(3)); }
	};
}