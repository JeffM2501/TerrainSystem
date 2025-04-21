#pragma once

#include "type_wrapper.h"
#include "attributes.h"

#include "type_values.h"

#include <memory>
#include <string>

namespace AssetTypes
{
    static constexpr std::string_view AssetReferenceEditor = "AssetReferenceEditor";
    
    class AssetRefTypeAttribute : public AttributeTypes::Attribute
    {
    public:
        DEFINE_ATTRIBUTE(AssetRefTypeAttribute)

            AssetRefTypeAttribute(std::string assetTypeId) : AssetTypeId(assetTypeId) {}

        std::string AssetTypeId;
    };

    class AssetReference : public TypeWraper
    {
    public:
        DEFINE_TYPE(AssetReference);

        static void Register()
        {
            auto* type = TypeDatabase::Get().CreateType(TypeName);
            type->AddAttribute<AttributeTypes::CustomEditorAttribute>(AssetReferenceEditor);

            type->AddPrimitiveField<std::string>("Path", std::string());
        }

        const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
        void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
        void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };
}