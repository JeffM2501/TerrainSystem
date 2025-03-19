#include "PropertyEditor.h"
#include "attributes.h"

using namespace Types;
using namespace AttributeTypes;

namespace Properties
{
    bool IntPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        return false;
    }

    bool FloatPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        return false;
    }

    bool StringPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        return false;
    }

    bool EnumPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        return false;
    }

    EditorRegistry::EditorRegistry()
    {
        BaseSet[IntEditorName] = IntPropertyEditor;
        BaseSet[FloatEditorName] = FloatPropertyEditor;
        BaseSet[StringEditorName] = StringPropertyEditor;
        BaseSet[EnumerationEditorName] = EnumPropertyEditor;
    }

    const char* GetPrimitveEditorName(PrimitiveType primType)
    {
        switch (primType)
        {
        case Types::PrimitiveType::Bool:
            break;

        case Types::PrimitiveType::Char:
        case Types::PrimitiveType::UInt8:
        case Types::PrimitiveType::UInt16:
        case Types::PrimitiveType::Int16:
        case Types::PrimitiveType::UInt32:
        case Types::PrimitiveType::Int32:
        case Types::PrimitiveType::UInt64:
        case Types::PrimitiveType::Int64:
            return IntEditorName;

        case Types::PrimitiveType::Float32:
        case Types::PrimitiveType::Double64:
            return FloatEditorName;

        case Types::PrimitiveType::String:
            return StringEditorName;

        case Types::PrimitiveType::Vector2:
            break;
        case Types::PrimitiveType::Vector3:
            break;
        case Types::PrimitiveType::Vector4:
            break;

        case Types::PrimitiveType::Rectangle:
            break;

        case Types::PrimitiveType::Matrix:
            break;

        case Types::PrimitiveType::GUID:
            break;

        case Types::PrimitiveType::Unknown:
        default:
            return nullptr;
        }

        return nullptr;
    }

    const char* GetNameForField(const Types::FieldInfo* fieldInfo)
    {
        switch (fieldInfo->GetType())
        {
        default:
            return nullptr;

        case FieldType::Primitive:
        case FieldType::PrimitiveList:
        {
            const PrimitiveFieldInfo* primitiveField = static_cast<const PrimitiveFieldInfo*>(fieldInfo);

            return GetPrimitveEditorName(primitiveField->GetPrimitiveType());
        }

        case FieldType::Type:
        case FieldType::TypeList:
        {
            const TypeFieldInfo* typeField = static_cast<const TypeFieldInfo*>(fieldInfo);

            return typeField->TypePtr->TypeName.c_str();
        }

        case FieldType::Enumeration:
            return EnumerationEditorName;
        }
    }


    EditorSet& EditorRegistry::PushSet()
    {
        SetStack.emplace_back();
        return SetStack.back();
    }

    void EditorRegistry::PopSet()
    {
        SetStack.pop_back();
    }

    PropertyEditor EditorRegistry::FindEditorByName(const std::string& name)
    {
        for (auto itr = SetStack.rbegin(); itr != SetStack.rend(); itr--)
        {
            auto editItr = itr->find(name);
            if (editItr != itr->end())
                return editItr->second;
        }

        auto editItr = BaseSet.find(name);
        if (editItr != BaseSet.end())
            return editItr->second;

        return nullptr;
    }

    PropertyEditor EditorRegistry::GetEditorForField(Types::TypeValue* value, int fieldIndex)
    {
        const Types::FieldInfo* fieldInfo = value->GetType()->GetField(fieldIndex);

        const auto* customEditor = value->GetFieldAttribute<CustomEditorAttribute>(fieldIndex);

        if (customEditor)
        {
            auto editor = FindEditorByName(customEditor->EditorName);
            if (editor)
                return editor;
        }

        return FindEditorByName(GetNameForField(fieldInfo));
    }
}