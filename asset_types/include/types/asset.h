#pragma once

#include "type_wrapper.h"

#include "GUID.h"

#include <memory>
#include <string>

using namespace Types;

namespace AssetTypes
{
    class ResourceReference : public TypeWraper
    {
    public:
        DEFINE_TYPE(ResourceReference);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<std::string>("Path", 0);
        }

        const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
        void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
        void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };

    class AssetReference : public TypeWraper
    {
    public:
        DEFINE_TYPE(AssetReference);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<Hashes::GUID>("GUID", Hashes::GUID::Invalid());
        }

        const Hashes::GUID& GetGUID() const { return ValuePtr->GetFieldPrimitiveValue<Hashes::GUID>(0); }
        void SetGUID(const Hashes::GUID& value) { ValuePtr->SetFieldPrimitiveValue<Hashes::GUID>(0, value); }
        void ResetGUID() { ValuePtr->ResetFieldToDefault(0); }
    };

    class AssetMetaInfo : public TypeWraper
    {
    public:
        DEFINE_TYPE(AssetMetaInfo);

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

    class Asset: public TypeWraper
    {
    public:
        DEFINE_TYPE(Asset);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddTypeField("Meta", AssetMetaInfo::TypeName);
        }

        AssetMetaInfo GetTestField() const { return AssetMetaInfo(ValuePtr->GetTypeFieldValue(0)); }
    };
}