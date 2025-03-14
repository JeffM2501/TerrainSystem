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
            type->AddPrimitiveField<std::string>("Path", 0);
        }

		const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
		void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
		void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };

    class Asset: public AssetReference
    {
    public:
        DEFINE_DERIVED_TYPE(Asset, AssetReference);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
        }
    };
}