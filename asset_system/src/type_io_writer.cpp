#include "type_io.h"

#include "field_info.h"
#include "attributes.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#include "raymath.h"

using namespace TypeIO;
using namespace rapidjson;


//--------------------------------------------------------------
//   File IO
//--------------------------------------------------------------

void WriteFileText(const std::string& fileName, const char* data, size_t size)
{
    std::string contents;
    FILE* fp = fopen(fileName.c_str(), "wt");
    if (!fp)
        return;

    fwrite(data, size, 1, fp);
    fclose(fp);
}

//--------------------------------------------------------------
//   TypeWriter
//--------------------------------------------------------------

bool TypeWriter::SetPrimitiveFieldJson(PrimitiveType primType, const FieldValue* value, rapidjson::Value& fieldType, rapidjson::Value& returnValue)
{
    switch (primType)
    {
    default:
    case PrimitiveType::Unknown:
        returnValue.SetString("UNKNOWN!!!!!!");
        return false;

    case PrimitiveType::Bool:
        fieldType.SetString("bool");
        returnValue.SetBool(static_cast<const PrimitiveFieldValue<bool>*>(value)->GetValue());
        break;
    case PrimitiveType::Char:
        fieldType.SetString("char");
        returnValue.SetInt(static_cast<const PrimitiveFieldValue<char>*>(value)->GetValue());
        break;
    case PrimitiveType::UInt8:
        fieldType.SetString("uint8");
        returnValue.SetUint(static_cast<const PrimitiveFieldValue<uint8_t>*>(value)->GetValue());
        break;
    case PrimitiveType::UInt16:
        fieldType.SetString("uint16");
        returnValue.SetUint(static_cast<const PrimitiveFieldValue<uint16_t>*>(value)->GetValue());
        break;
    case PrimitiveType::Int16:
        fieldType.SetString("int16");
        returnValue.SetInt(static_cast<const PrimitiveFieldValue<int16_t>*>(value)->GetValue());
        break;
    case PrimitiveType::UInt32:
        fieldType.SetString("uint32");
        returnValue.SetUint(static_cast<const PrimitiveFieldValue<uint32_t>*>(value)->GetValue());
        break;
    case PrimitiveType::Int32:
        fieldType.SetString("int32");
        returnValue.SetInt(static_cast<const PrimitiveFieldValue<int32_t>*>(value)->GetValue());
        break;
    case PrimitiveType::UInt64:
        fieldType.SetString("uint64");
        returnValue.SetUint64(static_cast<const PrimitiveFieldValue<uint64_t>*>(value)->GetValue());
        break;
    case PrimitiveType::Int64:
        fieldType.SetString("int64");
        returnValue.SetInt64(static_cast<const PrimitiveFieldValue<int64_t>*>(value)->GetValue());
        break;
    case PrimitiveType::Float32:
        fieldType.SetString("float");
        returnValue.SetFloat(static_cast<const PrimitiveFieldValue<float>*>(value)->GetValue());
        break;
    case PrimitiveType::Double64:
        fieldType.SetString("double");
        returnValue.SetDouble(static_cast<const PrimitiveFieldValue<double>*>(value)->GetValue());
        break;
    case PrimitiveType::String:
        fieldType.SetString("string");
        returnValue.SetString(StringRef(static_cast<const PrimitiveFieldValue<std::string>*>(value)->GetValue().c_str()));
        break;
    case PrimitiveType::Vector2:
    {
        fieldType.SetString("vector2");
        returnValue.SetObject();

        Vector2 vec = static_cast<const PrimitiveFieldValue<Vector2>*>(value)->GetValue();
        Value x;
        x.SetFloat(vec.x);
        returnValue.AddMember("X", x, RootDocument->GetAllocator());

        Value y;
        y.SetFloat(vec.y);
        returnValue.AddMember("Y", y, RootDocument->GetAllocator());
    }
    break;

    case PrimitiveType::Vector3:
    {
        fieldType.SetString("vector3");
        returnValue.SetObject();

        Vector3 vec = static_cast<const PrimitiveFieldValue<Vector3>*>(value)->GetValue();
        Value x;
        x.SetFloat(vec.x);
        returnValue.AddMember("X", x, RootDocument->GetAllocator());

        Value y;
        y.SetFloat(vec.y);
        returnValue.AddMember("Y", y, RootDocument->GetAllocator());

        Value z;
        z.SetFloat(vec.z);
        returnValue.AddMember("Z", z, RootDocument->GetAllocator());
    }
    break;

    case PrimitiveType::Vector4:
    {
        fieldType.SetString("vector4");
        returnValue.SetObject();

        Vector4 vec = static_cast<const PrimitiveFieldValue<Vector4>*>(value)->GetValue();
        Value x;
        x.SetFloat(vec.x);
        returnValue.AddMember("X", x, RootDocument->GetAllocator());

        Value y;
        y.SetFloat(vec.y);
        returnValue.AddMember("Y", y, RootDocument->GetAllocator());

        Value z;
        z.SetFloat(vec.z);
        returnValue.AddMember("Z", z, RootDocument->GetAllocator());

        Value w;
        w.SetFloat(vec.w);
        returnValue.AddMember("W", w, RootDocument->GetAllocator());
    }
    break;

    case PrimitiveType::Rectangle:
    {
        fieldType.SetString("rectangle");
        returnValue.SetObject();

        Rectangle rect = static_cast<const PrimitiveFieldValue<Rectangle>*>(value)->GetValue();
        Value x;
        x.SetFloat(rect.x);
        returnValue.AddMember("X", x, RootDocument->GetAllocator());

        Value y;
        y.SetFloat(rect.y);
        returnValue.AddMember("Y", y, RootDocument->GetAllocator());

        Value w;
        w.SetFloat(rect.width);
        returnValue.AddMember("Width", w, RootDocument->GetAllocator());

        Value h;
        h.SetFloat(rect.height);
        returnValue.AddMember("Height", h, RootDocument->GetAllocator());
    }
    break;

    case PrimitiveType::Matrix:
    {
        fieldType.SetString("matrix");
        returnValue.SetArray();

        Matrix mat = static_cast<const PrimitiveFieldValue<Matrix>*>(value)->GetValue();
        for (float v : MatrixToFloat(mat))
        {
            Value matValue;
            matValue.SetFloat(v);
            returnValue.PushBack(matValue, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::GUID:
        fieldType.SetString("GUID");
        returnValue.SetString(StringRef(static_cast<const PrimitiveFieldValue<Hashes::GUID>*>(value)->GetValue().ToString().c_str()));
        break;

    case PrimitiveType::Color:
    {
        fieldType.SetString("color");
        returnValue.SetArray();

        Color color = static_cast<const PrimitiveFieldValue<Color>*>(value)->GetValue();

        Value colorValue;
        colorValue.SetInt(color.r);
        returnValue.PushBack(colorValue, RootDocument->GetAllocator());

        colorValue.SetInt(color.g);
        returnValue.PushBack(colorValue, RootDocument->GetAllocator());

        colorValue.SetInt(color.b);
        returnValue.PushBack(colorValue, RootDocument->GetAllocator());

        colorValue.SetInt(color.a);
        returnValue.PushBack(colorValue, RootDocument->GetAllocator());
    }
    break;
    }

    return true;
}

bool TypeWriter::SetPrimitiveFieldListJson(PrimitiveType primType, const FieldValue* value, rapidjson::Value& fieldType, rapidjson::Value& returnValue)
{
    returnValue.SetArray();

    switch (primType)
    {
    default:
    case PrimitiveType::Unknown:
        returnValue.SetString("UNKNOWN!!!!!!");
        return false;

    case PrimitiveType::Bool:
        fieldType.SetString("bool[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<bool>*>(value))
        {
            Value v;
            v.SetBool(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Char:
        fieldType.SetString("char[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<char>*>(value))
        {
            Value v;
            v.SetInt(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::UInt8:
        fieldType.SetString("uint8[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<uint8_t>*>(value))
        {
            Value v;
            v.SetUint(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::UInt16:
        fieldType.SetString("uint16[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<uint16_t>*>(value))
        {
            Value v;
            v.SetUint(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Int16:
        fieldType.SetString("int16[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<int16_t>*>(value))
        {
            Value v;
            v.SetInt(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::UInt32:
        fieldType.SetString("uint32[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<uint32_t>*>(value))
        {
            Value v;
            v.SetUint(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Int32:
        fieldType.SetString("int32[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<int32_t>*>(value))
        {
            Value v;
            v.SetInt(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::UInt64:
        fieldType.SetString("uint64[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<uint64_t>*>(value))
        {
            Value v;
            v.SetUint64(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Int64:
        fieldType.SetString("int64[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<int64_t>*>(value))
        {
            Value v;
            v.SetInt64(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Float32:
        fieldType.SetString("float[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<float>*>(value))
        {
            Value v;
            v.SetFloat(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Double64:
        fieldType.SetString("double[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<double>*>(value))
        {
            Value v;
            v.SetDouble(value);
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::String:
        fieldType.SetString("string[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<std::string>*>(value))
        {
            Value v;
            v.SetString(StringRef(value.c_str()));
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;
    case PrimitiveType::Vector2:
    {
        fieldType.SetString("vector2[]");

        for (const auto& vec : *static_cast<const PrimitiveListFieldValue<Vector2>*>(value))
        {
            Value v;
            v.SetObject();
            Value x;
            x.SetFloat(vec.x);
            v.AddMember("X", x, RootDocument->GetAllocator());

            Value y;
            y.SetFloat(vec.y);
            v.AddMember("Y", y, RootDocument->GetAllocator());

            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::Vector3:
    {
        fieldType.SetString("vector3[]");

        for (auto& vec : *static_cast<const PrimitiveListFieldValue<Vector3>*>(value))
        {
            Value v;
            v.SetObject();
            Value x;
            x.SetFloat(vec.x);
            v.AddMember("X", x, RootDocument->GetAllocator());

            Value y;
            y.SetFloat(vec.y);
            v.AddMember("Y", y, RootDocument->GetAllocator());

            Value z;
            z.SetFloat(vec.z);
            v.AddMember("Z", z, RootDocument->GetAllocator());

            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::Vector4:
    {
        fieldType.SetString("vector4[]");
        for (auto& vec : *static_cast<const PrimitiveListFieldValue<Vector4>*>(value))
        {
            Value v;
            v.SetObject();
            Value x;
            x.SetFloat(vec.x);
            v.AddMember("X", x, RootDocument->GetAllocator());

            Value y;
            y.SetFloat(vec.y);
            v.AddMember("Y", y, RootDocument->GetAllocator());

            Value z;
            z.SetFloat(vec.z);
            v.AddMember("Z", z, RootDocument->GetAllocator());

            Value w;
            w.SetFloat(vec.w);
            v.AddMember("W", w, RootDocument->GetAllocator());

            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::Rectangle:
    {
        fieldType.SetString("rectangle[]");

        for (auto& vec : *static_cast<const PrimitiveListFieldValue<Rectangle>*>(value))
        {
            Value v;
            v.SetObject();
            Value x;
            x.SetFloat(vec.x);
            v.AddMember("X", x, RootDocument->GetAllocator());

            Value y;
            y.SetFloat(vec.y);
            v.AddMember("Y", y, RootDocument->GetAllocator());

            Value w;
            w.SetFloat(vec.width);
            v.AddMember("Width", w, RootDocument->GetAllocator());

            Value h;
            h.SetFloat(vec.height);
            v.AddMember("Height", h, RootDocument->GetAllocator());

            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::Matrix:
    {
        fieldType.SetString("matrix[]");

        for (auto& vec : *static_cast<const PrimitiveListFieldValue<Matrix>*>(value))
        {
            Value v;
            v.SetArray();
            Matrix mat = static_cast<const PrimitiveFieldValue<Matrix>*>(value)->GetValue();
            for (float m : MatrixToFloat(mat))
            {
                Value matValue;
                matValue.SetFloat(m);
                v.PushBack(matValue, RootDocument->GetAllocator());
            }
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;

    case PrimitiveType::GUID:
        fieldType.SetString("GUID[]");
        for (const auto& value : *static_cast<const PrimitiveListFieldValue<Hashes::GUID>*>(value))
        {
            Value v;
            v.SetString(StringRef(value.ToString().c_str()));
            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
        break;

    case PrimitiveType::Color:
    {
        fieldType.SetString("color[]");

        for (auto& vec : *static_cast<const PrimitiveListFieldValue<Color>*>(value))
        {
            Value v;
            v.SetArray();
            Color color = static_cast<const PrimitiveFieldValue<Color>*>(value)->GetValue();

            Value colorValue;
            colorValue.SetInt(color.r);
            v.PushBack(colorValue, RootDocument->GetAllocator());

            colorValue.SetInt(color.g);
            v.PushBack(colorValue, RootDocument->GetAllocator());

            colorValue.SetInt(color.b);
            v.PushBack(colorValue, RootDocument->GetAllocator());

            colorValue.SetInt(color.a);
            v.PushBack(colorValue, RootDocument->GetAllocator());

            returnValue.PushBack(v, RootDocument->GetAllocator());
        }
    }
    break;
    }

    return true;
}

Value TypeWriter::WritePrimitiveField(const FieldInfo* fieldInfo, const FieldValue* value)
{
    const PrimitiveFieldInfo* primField = static_cast<const PrimitiveFieldInfo*>(fieldInfo);

    Value fieldValue;
    fieldValue.SetObject();

    Value fieldType;
    Value returnValue;

    if (fieldInfo->GetType() == FieldType::PrimitiveList)
    {
        if (SetPrimitiveFieldListJson(primField->GetPrimitiveType(), value, fieldType, returnValue))
        {
            fieldValue.AddMember(TypeName, fieldType, RootDocument->GetAllocator());
            fieldValue.AddMember(ValueName, returnValue, RootDocument->GetAllocator());
        }
    }
    else
    {
        if (SetPrimitiveFieldJson(primField->GetPrimitiveType(), value, fieldType, returnValue))
        {
            fieldValue.AddMember(TypeName, fieldType, RootDocument->GetAllocator());
            fieldValue.AddMember(ValueName, returnValue, RootDocument->GetAllocator());
        }
    }
    return fieldValue;
}

Value TypeWriter::WriteTypeValue(const TypeValue* value)
{
    Value  rootType;
    rootType.SetObject();

    const TypeInfo* type = value->GetType();
    if (!type)
        return rootType;

    Value typeObjet;
    typeObjet.SetString(type->TypeName.c_str(), RootDocument->GetAllocator());
    rootType.AddMember(TypeName, typeObjet, RootDocument->GetAllocator());

    Value fieldList;
    fieldList.SetObject();
    for (const auto& [index, fieldValue] : value->GetTypeFieldValues())
    {
        Value fieldObject;

        const FieldInfo* fieldInfo = type->GetField(index);
        FieldType fieldType = fieldInfo->GetType();

        switch (fieldType)
        {
        case Types::FieldType::Primitive:
        case Types::FieldType::PrimitiveList:
        {
            fieldObject = WritePrimitiveField(fieldInfo, fieldValue.get());
        }
        break;

        case Types::FieldType::Type:
        {
            fieldObject = WriteTypeValue(static_cast<const TypeValue*>(fieldValue.get()));
        }
        break;

        case Types::FieldType::TypeList:
        {
            fieldObject = WriteTypeListValue(static_cast<const TypeListValue*>(fieldValue.get()));
        }
        break;

        default:
            continue;
            break;
        }

        fieldList.AddMember(StringRef(fieldInfo->GetName().c_str()), fieldObject, RootDocument->GetAllocator());

    }
    rootType.AddMember(Fields, fieldList, RootDocument->GetAllocator());
    return rootType;
}

rapidjson::Value TypeWriter::WriteTypeListValue(const TypeListValue* value)
{
    Value  rootType;
    rootType.SetObject();
    const TypeInfo* type = value->GetType();
    if (!type)
        return rootType;

    Value typeObjet;
    typeObjet.SetString(type->TypeName.c_str(), RootDocument->GetAllocator());
    rootType.AddMember(TypeName, typeObjet, RootDocument->GetAllocator());

    Value returnValue;
    returnValue.SetArray();

    for (size_t i = 0; i < value->Size(); i++)
    {
        if (type->FieldHasAttribute<AttributeTypes::NoSerializationAttribute>(int(i)))
            continue;

        returnValue.PushBack(WriteTypeValue(&value->Get(i)), RootDocument->GetAllocator());
    }
    rootType.AddMember(ValueName, returnValue, RootDocument->GetAllocator());
    return rootType;
}

bool TypeWriter::Write(TypeValue* value, const std::string& fileName)
{
    Document doc;
    RootDocument = &doc;

    doc.SetObject();
    // version info
    Value  versionNumber;
    versionNumber.SetString("v1", doc.GetAllocator());
    doc.AddMember(Version, versionNumber, doc.GetAllocator());

    // TODO, write meta

    doc.AddMember(RootData, WriteTypeValue(value), doc.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetMaxDecimalPlaces(3);

    doc.Accept(writer);
    WriteFileText(fileName.c_str(), buffer.GetString(), buffer.GetSize());

    return true;
}
