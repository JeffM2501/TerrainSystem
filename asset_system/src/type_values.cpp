#include "type_values.h"

using namespace Types;


namespace PrimitiveFieldFactory
{
	std::unique_ptr<FieldValue> Create(PrimitiveType primitiveType)
	{
		switch (primitiveType)
		{
		default:
		case Types::PrimitiveType::Unknown:
			return nullptr;
		case Types::PrimitiveType::Bool:
			return std::make_unique<PrimitiveFieldValue<bool>>();
		case Types::PrimitiveType::Char:
			return std::make_unique<PrimitiveFieldValue<char>>();
		case Types::PrimitiveType::UInt8:
			return std::make_unique<PrimitiveFieldValue<uint8_t>>();
		case Types::PrimitiveType::UInt16:
			return std::make_unique<PrimitiveFieldValue<uint16_t>>();
		case Types::PrimitiveType::Int16:
			return std::make_unique<PrimitiveFieldValue<int16_t>>();
		case Types::PrimitiveType::UInt32:
			return std::make_unique<PrimitiveFieldValue<uint32_t>>();
		case Types::PrimitiveType::Int32:
			return std::make_unique<PrimitiveFieldValue<int32_t>>();
		case Types::PrimitiveType::UInt64:
			return std::make_unique<PrimitiveFieldValue<uint64_t>>();
		case Types::PrimitiveType::Int64:
			return std::make_unique<PrimitiveFieldValue<int64_t>>();
		case Types::PrimitiveType::Float32:
			return std::make_unique<PrimitiveFieldValue<float>>();
		case Types::PrimitiveType::Double64:
			return std::make_unique<PrimitiveFieldValue<double>>();
		case Types::PrimitiveType::String:
			return std::make_unique<PrimitiveFieldValue<std::string>>();
		case Types::PrimitiveType::Vector2:
			return std::make_unique<PrimitiveFieldValue<Vector2>>();
		case Types::PrimitiveType::Vector3:
			return std::make_unique<PrimitiveFieldValue<Vector3>>();
		case Types::PrimitiveType::Vector4:
			return std::make_unique<PrimitiveFieldValue<Vector4>>();
		case Types::PrimitiveType::Rectangle:
			return std::make_unique<PrimitiveFieldValue<Rectangle>>();
		case Types::PrimitiveType::Matrix:
			return std::make_unique<PrimitiveFieldValue<Matrix>>();
		case Types::PrimitiveType::GUID:
			return std::make_unique<PrimitiveFieldValue<Hashes::GUID>>();
        case Types::PrimitiveType::Color:
            return std::make_unique<PrimitiveFieldValue<Color>>();
		}
	}
}

std::unique_ptr<ListFieldValue> ListFieldValue::Create(PrimitiveType primitiveType)
{
	switch (primitiveType)
	{
	default:
	case Types::PrimitiveType::Unknown:
		return nullptr;
	case Types::PrimitiveType::Bool:
		return std::make_unique<PrimitiveListFieldValue<bool>>();
	case Types::PrimitiveType::Char:
		return std::make_unique<PrimitiveListFieldValue<char>>();
	case Types::PrimitiveType::UInt8:
		return std::make_unique<PrimitiveListFieldValue<uint8_t>>();
	case Types::PrimitiveType::UInt16:
		return std::make_unique<PrimitiveListFieldValue<uint16_t>>();
	case Types::PrimitiveType::Int16:
		return std::make_unique<PrimitiveListFieldValue<int16_t>>();
	case Types::PrimitiveType::UInt32:
		return std::make_unique<PrimitiveListFieldValue<uint32_t>>();
	case Types::PrimitiveType::Int32:
		return std::make_unique<PrimitiveListFieldValue<int32_t>>();
	case Types::PrimitiveType::UInt64:
		return std::make_unique<PrimitiveListFieldValue<uint64_t>>();
	case Types::PrimitiveType::Int64:
		return std::make_unique<PrimitiveListFieldValue<int64_t>>();
	case Types::PrimitiveType::Float32:
		return std::make_unique<PrimitiveListFieldValue<float>>();
	case Types::PrimitiveType::Double64:
		return std::make_unique<PrimitiveListFieldValue<double>>();
	case Types::PrimitiveType::String:
		return std::make_unique<PrimitiveListFieldValue<std::string>>();
	case Types::PrimitiveType::Vector2:
		return std::make_unique<PrimitiveListFieldValue<Vector2>>();
	case Types::PrimitiveType::Vector3:
		return std::make_unique<PrimitiveListFieldValue<Vector3>>();
	case Types::PrimitiveType::Vector4:
		return std::make_unique<PrimitiveListFieldValue<Vector4>>();
	case Types::PrimitiveType::Rectangle:
		return std::make_unique<PrimitiveListFieldValue<Rectangle>>();
	case Types::PrimitiveType::Matrix:
		return std::make_unique<PrimitiveListFieldValue<Matrix>>();
	case Types::PrimitiveType::GUID:
		return std::make_unique<PrimitiveListFieldValue<Hashes::GUID>>();
    case Types::PrimitiveType::Color:
        return std::make_unique<PrimitiveListFieldValue<Color>>();
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

		std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(fieldPtr->TypePtr);

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

		std::unique_ptr<TypeListValue> value = std::make_unique<TypeListValue>(fieldPtr->TypePtr);

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

	std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(type);
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
					std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>(field->DefaultPtrType);
					Values.insert_or_assign(index, std::move(value)).first;
				}
			}
		}
	}
}
