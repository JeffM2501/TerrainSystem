#include "type_values.h"

using namespace Types;


std::unique_ptr<ListFieldValue> ListFieldValue::Create(PrimitiveType primitiveType, TypeValue* parentValue, const FieldPath& path)
{
    switch (primitiveType)
    {
    default:
    case Types::PrimitiveType::Unknown:
        return nullptr;
    case Types::PrimitiveType::Bool:
        return std::make_unique<PrimitiveListFieldValue<bool>>(parentValue, path);
    case Types::PrimitiveType::Char:
        return std::make_unique<PrimitiveListFieldValue<char>>(parentValue, path);
    case Types::PrimitiveType::UInt8:
        return std::make_unique<PrimitiveListFieldValue<uint8_t>>(parentValue, path);
    case Types::PrimitiveType::UInt16:
        return std::make_unique<PrimitiveListFieldValue<uint16_t>>(parentValue, path);
    case Types::PrimitiveType::Int16:
        return std::make_unique<PrimitiveListFieldValue<int16_t>>(parentValue, path);
    case Types::PrimitiveType::UInt32:
        return std::make_unique<PrimitiveListFieldValue<uint32_t>>(parentValue, path);
    case Types::PrimitiveType::Int32:
        return std::make_unique<PrimitiveListFieldValue<int32_t>>(parentValue, path);
    case Types::PrimitiveType::UInt64:
        return std::make_unique<PrimitiveListFieldValue<uint64_t>>(parentValue, path);
    case Types::PrimitiveType::Int64:
        return std::make_unique<PrimitiveListFieldValue<int64_t>>(parentValue, path);
    case Types::PrimitiveType::Float32:
        return std::make_unique<PrimitiveListFieldValue<float>>(parentValue, path);
    case Types::PrimitiveType::Double64:
        return std::make_unique<PrimitiveListFieldValue<double>>(parentValue, path);
    case Types::PrimitiveType::String:
        return std::make_unique<PrimitiveListFieldValue<std::string>>(parentValue, path);
    case Types::PrimitiveType::Vector2:
        return std::make_unique<PrimitiveListFieldValue<Vector2>>(parentValue, path);
    case Types::PrimitiveType::Vector3:
        return std::make_unique<PrimitiveListFieldValue<Vector3>>(parentValue, path);
    case Types::PrimitiveType::Vector4:
        return std::make_unique<PrimitiveListFieldValue<Vector4>>(parentValue, path);
    case Types::PrimitiveType::Rectangle:
        return std::make_unique<PrimitiveListFieldValue<Rectangle>>(parentValue, path);
    case Types::PrimitiveType::Matrix:
        return std::make_unique<PrimitiveListFieldValue<Matrix>>(parentValue, path);
    case Types::PrimitiveType::GUID:
        return std::make_unique<PrimitiveListFieldValue<Hashes::GUID>>(parentValue, path);
    case Types::PrimitiveType::Color:
        return std::make_unique<PrimitiveListFieldValue<Color>>(parentValue, path);
    }
}

void TypeValue::ResetFieldToDefault(int fieldIndex)
{
    auto itr = Values.find(fieldIndex);
    if (itr != Values.end())
        Values.erase(itr);
}

TypeValue* TypeValue::GetTypeFieldValue(int fieldIndex)
{
    auto itr = Values.find(fieldIndex);
    if (itr == Values.end())
    {
        const TypeFieldInfo* fieldPtr = Type->GetField<TypeFieldInfo>(fieldIndex);

        if (fieldPtr->IsPointer)
            return nullptr;

        std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(fieldPtr->TypePtr, this, FieldPath::Field(fieldIndex));

        itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
    }

    return (TypeValue*)itr->second.get();
}

TypeListValue& TypeValue::GetTypeListFieldValue(int fieldIndex)
{
    auto itr = Values.find(fieldIndex);
    if (itr == Values.end())
    {
        const TypeListFieldInfo* fieldPtr = Type->GetField<TypeListFieldInfo>(fieldIndex);

        std::unique_ptr<TypeListValue> value = std::make_unique<TypeListValue>(fieldPtr->TypePtr, this, FieldPath::Field(fieldIndex));

        itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
    }

    return *(TypeListValue*)itr->second.get();
}

TypeValue* TypeValue::SetTypeFieldPointer(const TypeInfo* type, int fieldIndex)
{
    auto itr = Values.find(fieldIndex);
    if (itr != Values.end())
    {
        TypeValue* value = (TypeValue*)itr->second.get();
        if (value->GetType() == type)
            return value;

        Values.erase(itr);
    }

    FieldPath path(FieldPath::Field(fieldIndex));
    std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(type, this, path);
    itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;

    return (TypeValue*)itr->second.get();
}

TypeValue* TypeValue::SetTypeFieldPointer(const std::string& typeName, int fieldIndex)
{
    return SetTypeFieldPointer(TypeDatabase::Get().FindType(typeName), fieldIndex);
}

void TypeValue::SetType(const TypeInfo* type)
{
    Type = type;
    Values.clear();

    if (type)
    {
        for (int index = 0; index < Type->GetFieldCount(); index++)
        {
            if (type->GetField(index)->GetType() == FieldType::Type)
            {
                const TypeFieldInfo* field = Type->GetField<TypeFieldInfo>(index);
                if (field->IsPointer && field->DefaultPtrType)
                {
                    std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(field->DefaultPtrType, this, FieldPath::Field(index));
                    Values.insert_or_assign(index, std::move(value)).first;
                }
            }
        }
    }
}

std::string_view TypeValue::GetFieldNameFromPath(const FieldPath& path, int pathIndex /*= 0*/)
{
    int fieldIndex = path.Elements[pathIndex].Index;

    auto* fieldInfo = GetType()->GetField(fieldIndex);

    // end of the path, we know the name
    if (pathIndex == path.Elements.size() - 1)
        return fieldInfo->GetName();

    if (fieldInfo->IsPrimtive() || fieldInfo->IsEnum())
    {
        // anything at the end of the path is pointless as it has no fields
        return fieldInfo->GetName();
    }
    else
    {
        if (fieldInfo->IsList())
        {
            pathIndex++;
            auto listIndex = path.Elements[pathIndex].Index;

            auto& listField = GetTypeListFieldValue(fieldIndex);
            if (listIndex >= listField.Size())
                return fieldInfo->GetName();

            return listField[listIndex].GetFieldNameFromPath(path, pathIndex + 1);
        }

        const TypeFieldInfo* typeFieldInfo = static_cast<const TypeFieldInfo*>(fieldInfo);
        return typeFieldInfo->TypePtr->GetFieldNameFromPath(path, pathIndex + 1);
    }
    return std::string_view();
}
