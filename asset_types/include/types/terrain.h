#pragma once

#include "type_wrapper.h"

#include "types/asset.h"
#include "raylib.h"

#include <memory>

using namespace Types;

namespace AssetTypes
{
    class TerrainPosition : public TypeWraper
    {
    public:
        DEFINE_TYPE(TerrainPosition);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<int64_t>("X", 0);
            type->AddPrimitiveField<int64_t>("Y", 1);
        }

        const float& GetX() const { return ValuePtr->GetFieldPrimitiveValue<int64_t>(0); }
        void SetX(const int64_t& value) { ValuePtr->SetFieldPrimitiveValue<int64_t>(0, value); }
        void ResetX() { ValuePtr->ResetFieldToDefault(0); }

        const int64_t& GetY() const { return ValuePtr->GetFieldPrimitiveValue<int64_t>(1); }
        void SetY(const int64_t& value) { ValuePtr->SetFieldPrimitiveValue<int64_t>(1, value); }
        void ResetY() { Value->ResetFieldToDefault(1); }
    };

    class TerrainMaterial : public TypeWraper
    {
    public:
        DEFINE_TYPE(TerrainMaterial);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddTypeField("DiffuseTexture", ResourceReference::TypeName);
            type->AddPrimitiveField<Vector4>("DiffuseColor", Vector4{1,1,1,1});
            type->AddTypeField("NormalMap", ResourceReference::TypeName);
        }

        ResourceReference GetDiffuseTexture() const { return ResourceReference(ValuePtr->GetTypeFieldValue(0)); }

        const Vector4& GetDiffuseColor() const { return ValuePtr->GetFieldPrimitiveValue<Vector4>(1); }
        void SetDiffuseColor(const Vector4& value) { ValuePtr->SetFieldPrimitiveValue<Vector4>(1, value); }
        void ResetDiffuseColor() { Value->ResetFieldToDefault(1); }

        ResourceReference GetNormalMap() const { return ResourceReference(ValuePtr->GetTypeFieldValue(2)); }
    };

    struct TerrainInfo
    {
        uint8_t TerrainGridSize = 128;

        float TerrainTileSize = 128;
        float TerrainMinZ = -50;
        float TerrainMaxZ = 100;
    };

    class TerrainInfo : public TypeWraper
    {
    public:
        DEFINE_TYPE(TerrainInfo);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<float>("TestFloat", 123.456f);
            type->AddPrimitiveField<int32_t>("TestInt", 123);
            type->AddPrimitiveField<bool>("TestBool", false);
            type->AddPrimitiveField<std::string>("TestString", "Default_Value");
        }

        const float& GetTestFloat() const { return ValuePtr->GetFieldPrimitiveValue<float>(0); }
        void SetTestFloat(const float& value) { ValuePtr->SetFieldPrimitiveValue<float>(0, value); }
        void ResetTestFloat() { ValuePtr->ResetFieldToDefault(0); }

        const int32_t& GetTestInt() const { return ValuePtr->GetFieldPrimitiveValue<int32_t>(1); }
        void SetTestInt(const int32_t& value) { ValuePtr->SetFieldPrimitiveValue<int32_t>(1, value); }
        void ResetTestInt() { Value->ResetFieldToDefault(1); }

        const bool& GetTestBool() const { return ValuePtr->GetFieldPrimitiveValue<bool>(2); }
        void SetTestBool(const bool& value) { ValuePtr->SetFieldPrimitiveValue<bool>(2, value); }
        void ResetTestBool() { ValuePtr->ResetFieldToDefault(2); }

        const std::string& GetTestString() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(3); }
        void SetTestString(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(3, value); }
        void ResetTestString() { ValuePtr->ResetFieldToDefault(3); }
    };

    class TerrainAsset : public Asset
    {
    public:
        DEFINE_TYPE(TerrainAsset);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<Hashes::GUID>("GUID", Hashes::GUID::Invalid());
            type->AddPrimitiveField<uint64_t>("TypeId", 0);
            type->AddPrimitiveField<std::string>("TypeName", "");
        }

        const Hashes::GUID& GetGUID() const { return ValuePtr->GetFieldPrimitiveValue<Hashes::GUID>(0); }
        void SetGUID(const Hashes::GUID& value) { ValuePtr->SetFieldPrimitiveValue<Hashes::GUID>(0, value); }
        void ResetGUID() { ValuePtr->ResetFieldToDefault(0); }

        const uint64_t& GetTypeId() const { return ValuePtr->GetFieldPrimitiveValue<uint64_t>(1); }
        void SetTypeId(const uint64_t& value) { ValuePtr->SetFieldPrimitiveValue<uint64_t>(1, value); }
        void ResetTypeId() { ValuePtr->ResetFieldToDefault(1); }

        const std::string& GetTypeName() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(2); }
        void SetTypeName(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(2, value); }
        void ResetTypeNameD() { ValuePtr->ResetFieldToDefault(2); }
    };
}