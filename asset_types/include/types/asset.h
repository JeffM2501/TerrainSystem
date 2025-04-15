#pragma once

#include "type_wrapper.h"
#include "attributes.h"
#include "GUID.h"

#include <memory>
#include <string>

using namespace Types;

namespace AssetTypes
{
    static constexpr std::string_view ResourceReferenceEditor = "ResourceReferenceEditor";
    static constexpr std::string_view TextureReferenceEditor = "TextureReferenceEditor";

    // attributes
    class AssetPathAttribute : public AttributeTypes::Attribute
    {
    public:
        DEFINE_ATTRIBUTE(AssetPathAttribute)
    };

    class FileExtensionFilterAttribute : public AttributeTypes::Attribute
    {
    public:
        DEFINE_ATTRIBUTE(FileExtensionFilterAttribute)

            std::vector<std::string> Filters;
    };

    class FileExtensionAttribute : public AttributeTypes::Attribute
    {
    public:
        DEFINE_ATTRIBUTE(FileExtensionFilterAttribute)

            FileExtensionAttribute(std::string_view ext) : Extension(ext) {}

        std::string Extension;
    };

    // types
    class ResourceReference : public TypeWraper
    {
    public:
        DEFINE_TYPE(ResourceReference);

        static void Register()
        {
            auto* type = TypeDatabase::Get().CreateType(TypeName);
            type->AddAttribute<AttributeTypes::CustomEditorAttribute>(ResourceReferenceEditor);

            type->AddPrimitiveField<std::string>("Path", std::string());
        }

        const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
        void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
        void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };

    class AssetReference : public TypeWraper
    {
    public:
        DEFINE_TYPE(AssetReference);

        static void Register()
        {
            auto* type = TypeDatabase::Get().CreateType(TypeName);
            type->AddAttribute<AttributeTypes::CustomEditorAttribute>(ResourceReferenceEditor);

            type->AddPrimitiveField<std::string>("Path", std::string());
        }

        const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
        void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
        void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };

    class Asset : public TypeWraper
    {
    public:
        DEFINE_TYPE(Asset);

        static void Register()
        {
            auto* type = TypeDatabase::Get().CreateType(TypeName);

            auto path = type->AddPrimitiveField<std::string>("Path", std::string());
            path->AddAttribute<AttributeTypes::NoSerializationAttribute>();
            path->AddAttribute<AttributeTypes::ReadOnlyAttribute>();
            path->AddAttribute<AssetPathAttribute>();
        }

        const std::string& GetPath() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(0); }
        void SetPath(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(0, value); }
        void ResetPath() { ValuePtr->ResetFieldToDefault(0); }
    };
}