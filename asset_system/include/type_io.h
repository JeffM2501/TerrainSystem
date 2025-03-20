#pragma once

#include "type_database.h"
#include "type_values.h"

#include "rapidjson/document.h"

#include <string>

using namespace Types;

namespace TypeIO
{
	constexpr char Version[] = "Version";
	constexpr char RootData[] = "RootData";
	constexpr char TypeName[] = "TypeName";
	constexpr char Fields[] = "Fields";
	constexpr char ValueName[] = "Value";

	class TypeReader
	{
	protected:
		rapidjson::Document* RootDocument = nullptr;

		bool AddPrimtiveField(TypeValue* destinationValue, int fieldIndex, PrimitiveFieldInfo* fieldInfo, rapidjson::Value& jsonValue);
		bool AddPrimtiveListField(TypeValue* destinationValue, int fieldIndex, PrimitiveFieldInfo* fieldInfo, rapidjson::Value& jsonValue);

		bool ReadTypeValue(TypeValue* destinationValue, rapidjson::Value& jsonValue);
		bool ReadTypeListValue(TypeListValue& listValue, rapidjson::Value& jsonValue);

	public:
		bool Read(TypeValue* value, const std::string& fileName);

	};

	class TypeWriter
	{
	protected:
		rapidjson::Document* RootDocument = nullptr;

		rapidjson::Value WriteTypeValue(const TypeValue* value);
		rapidjson::Value WriteTypeListValue(const TypeListValue* value);

		rapidjson::Value WritePrimitiveField(const FieldInfo* fieldInfo, const FieldValue* value);

		bool SetPrimitiveFieldJson(PrimitiveType primType, const FieldValue* value, rapidjson::Value& fieldType, rapidjson::Value& fieldValue);
		bool SetPrimitiveFieldListJson(PrimitiveType primType, const FieldValue* value, rapidjson::Value& fieldType, rapidjson::Value& fieldValue);

	public:
		bool Write(TypeValue* value, const std::string& fileName);
	};
}