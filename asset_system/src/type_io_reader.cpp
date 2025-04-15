#include "type_io.h"

#include "field_info.h"

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
std::string ReadFileText(const std::string& fileName)
{
    std::string contents;
    FILE* fp = fopen(fileName.c_str(), "rt");
    if (!fp)
        return contents;

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    contents.resize(len + 1);
    fread(contents.data(), len, 1, fp);
    fclose(fp);

    return contents;
}

//--------------------------------------------------------------
//   TypeReader
//--------------------------------------------------------------

Matrix FloatToMatrix(float mat[16])
{
    Matrix result = { 0 };

    result.m0 = mat[0];
    result.m1 = mat[1];
    result.m2 = mat[2];
    result.m3 = mat[3];
    result.m4 = mat[4];
    result.m5 = mat[5];
    result.m6 = mat[6];
    result.m7 = mat[7];
    result.m8 = mat[8];
    result.m9 = mat[9];
    result.m10 = mat[10];
    result.m11 = mat[11];
    result.m12 = mat[12];
    result.m13 = mat[13];
    result.m14 = mat[14];
    result.m15 = mat[15];

    return result;
}

bool TypeReader::AddPrimtiveField(TypeValue* destinationValue, int fieldIndex, PrimitiveFieldInfo* fieldInfo, rapidjson::Value& jsonValue)
{
    if (!jsonValue.IsObject())
        return false;

    rapidjson::Value& typeName = jsonValue[TypeName];
    rapidjson::Value& fieldValue = jsonValue[ValueName];

    switch (fieldInfo->GetPrimitiveType())
    {
    default:
    case PrimitiveType::Unknown:
        return false;

    case PrimitiveType::Bool:
        destinationValue->SetFieldPrimitiveValue<bool>(fieldIndex, fieldValue.GetBool());
        break;
    case PrimitiveType::Char:
        destinationValue->SetFieldPrimitiveValue<char>(fieldIndex, fieldValue.GetInt());
        break;
    case PrimitiveType::UInt8:
        destinationValue->SetFieldPrimitiveValue<uint8_t>(fieldIndex, fieldValue.GetUint());
        break;
    case PrimitiveType::UInt16:
        destinationValue->SetFieldPrimitiveValue<uint16_t>(fieldIndex, fieldValue.GetUint());
        break;
    case PrimitiveType::Int16:
        destinationValue->SetFieldPrimitiveValue<int16_t>(fieldIndex, fieldValue.GetInt());
        break;
    case PrimitiveType::UInt32:
        destinationValue->SetFieldPrimitiveValue<uint32_t>(fieldIndex, fieldValue.GetUint());
        break;
    case PrimitiveType::Int32:
        destinationValue->SetFieldPrimitiveValue<int32_t>(fieldIndex, fieldValue.GetInt());
        break;
    case PrimitiveType::UInt64:
        destinationValue->SetFieldPrimitiveValue<uint64_t>(fieldIndex, fieldValue.GetUint64());
        break;
    case PrimitiveType::Int64:
        destinationValue->SetFieldPrimitiveValue<int64_t>(fieldIndex, fieldValue.GetInt64());
        break;
    case PrimitiveType::Float32:
        destinationValue->SetFieldPrimitiveValue<float>(fieldIndex, fieldValue.GetFloat());
        break;
    case PrimitiveType::Double64:
        destinationValue->SetFieldPrimitiveValue<double>(fieldIndex, fieldValue.GetDouble());
        break;
    case PrimitiveType::String:
        destinationValue->SetFieldPrimitiveValue<std::string>(fieldIndex, fieldValue.GetString());
        break;
    case PrimitiveType::Vector2:
    {
        auto& x = fieldValue["X"];
        auto& y = fieldValue["Y"];
        destinationValue->SetFieldPrimitiveValue<Vector2>(fieldIndex, Vector2{ x.GetFloat(), y.GetFloat() });
    }
    break;
    case PrimitiveType::Vector3:
    {
        auto& x = fieldValue["X"];
        auto& y = fieldValue["Y"];
        auto& z = fieldValue["Z"];
        destinationValue->SetFieldPrimitiveValue<Vector3>(fieldIndex, Vector3{ x.GetFloat(), y.GetFloat(), z.GetFloat() });
    }
    break;
    case PrimitiveType::Vector4:
    {
        auto& x = fieldValue["X"];
        auto& y = fieldValue["Y"];
        auto& z = fieldValue["Z"];
        auto& w = fieldValue["W"];
        destinationValue->SetFieldPrimitiveValue<Vector4>(fieldIndex, Vector4{ x.GetFloat(), y.GetFloat(), z.GetFloat(), w.GetFloat() });
    }
    break;
    case PrimitiveType::Rectangle:
    {
        auto& x = fieldValue["X"];
        auto& y = fieldValue["Y"];
        auto& w = fieldValue["Width"];
        auto& h = fieldValue["Height"];
        destinationValue->SetFieldPrimitiveValue<Rectangle>(fieldIndex, Rectangle{ x.GetFloat(), y.GetFloat(), w.GetFloat(), h.GetFloat() });
    }
    break;
    case PrimitiveType::Matrix:
    {
        float matf[16] = { 0 };
        const auto& array = fieldValue.GetArray();

        for (int i = 0; i < 16; i++)
        {
            matf[i] = array[i].GetFloat();
        }
        Matrix mat = FloatToMatrix(matf);
        destinationValue->SetFieldPrimitiveValue<Matrix>(fieldIndex, mat);
    }
    break;
    case PrimitiveType::GUID:
    {
        Hashes::GUID guid;
        guid.Parse(fieldValue.GetString());
        destinationValue->SetFieldPrimitiveValue<Hashes::GUID>(fieldIndex, guid);
    }
    break;
    case PrimitiveType::Color:
    {
        Color color;
        const auto& array = fieldValue.GetArray();
        color.r = array[0].GetInt();
        color.g = array[1].GetInt();
        color.b = array[2].GetInt();
        color.a = array[3].GetInt();
        destinationValue->SetFieldPrimitiveValue<Color>(fieldIndex, color);
    }
    break;
    }

    return true;
}

bool TypeReader::AddPrimtiveListField(TypeValue* destinationValue, int fieldIndex, PrimitiveFieldInfo* fieldInfo, rapidjson::Value& jsonValue)
{
    if (!jsonValue.IsObject())
        return false;

    rapidjson::Value& typeName = jsonValue[TypeName];
    rapidjson::Value& fieldValue = jsonValue[ValueName];

    if (!fieldValue.IsArray())
        return false;

    const auto& array = fieldValue.GetArray();

    switch (fieldInfo->GetPrimitiveType())
    {
    default:
    case PrimitiveType::Unknown:
        return false;

    case PrimitiveType::Bool:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<bool>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetBool());
        }
    }
    break;

    case PrimitiveType::Char:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<char>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetInt());
        }
    }
    break;

    case PrimitiveType::UInt8:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<uint8_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetUint());
        }
    }
    break;

    case PrimitiveType::UInt16:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<uint16_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetUint());
        }
    }
    break;

    case PrimitiveType::Int16:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<int16_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetInt());
        }
    }
    break;

    case PrimitiveType::UInt32:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<uint32_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetUint());
        }
    }
    break;

    case PrimitiveType::Int32:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<int32_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetInt());
        }
    }
    break;

    case PrimitiveType::UInt64:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<uint64_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetUint64());
        }
    }
    break;

    case PrimitiveType::Int64:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<int64_t>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetInt64());
        }
    }
    break;

    case PrimitiveType::Float32:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<float>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetFloat());
        }
    }
    break;

    case PrimitiveType::Double64:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<double>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetDouble());
        }
    }
    break;

    case PrimitiveType::String:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<std::string>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(i.GetString());
        }
    }
    break;

    case PrimitiveType::Vector2:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Vector2>(fieldIndex);
        for (auto& i : array)
        {
            auto& x = i["X"];
            auto& y = i["Y"];
            list.PushBack(Vector2{ x.GetFloat(), y.GetFloat() });
        }
    }
    break;

    case PrimitiveType::Vector3:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Vector3>(fieldIndex);
        for (auto& i : array)
        {
            auto& x = i["X"];
            auto& y = i["Y"];
            auto& z = i["Z"];
            list.PushBack(Vector3{ x.GetFloat(), y.GetFloat(), z.GetFloat() });
        }
    }
    break;

    case PrimitiveType::Vector4:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Vector4>(fieldIndex);
        for (auto& i : array)
        {
            auto& x = i["X"];
            auto& y = i["Y"];
            auto& z = i["Z"];
            auto& w = i["W"];
            list.PushBack(Vector4{ x.GetFloat(), y.GetFloat(), z.GetFloat(), w.GetFloat() });
        }
    }
    break;

    case PrimitiveType::Rectangle:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Rectangle>(fieldIndex);
        for (auto& i : array)
        {
            auto& x = i["X"];
            auto& y = i["Y"];
            auto& w = i["Width"];
            auto& h = i["Height"];
            list.PushBack(Rectangle{ x.GetFloat(), y.GetFloat(), w.GetFloat(), h.GetFloat() });
        }
    }
    break;

    case PrimitiveType::Matrix:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Matrix>(fieldIndex);
        for (auto& i : array)
        {
            float matf[16] = { 0 };
            const auto& matArray = i.GetArray();

            for (int m = 0; m < 16; m++)
            {
                matf[m] = matArray[m].GetFloat();
            }
            Matrix mat = FloatToMatrix(matf);
            list.PushBack(mat);
        }
    }
    break;

    case PrimitiveType::GUID:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Hashes::GUID>(fieldIndex);
        for (auto& i : array)
        {
            list.PushBack(Hashes::GUID::FromString(i.GetString()));
        }
    }
    break;

    case PrimitiveType::Color:
    {
        auto& list = destinationValue->GetPrimitiveListFieldValue<Color>(fieldIndex);
        for (auto& i : array)
        {
            float matf[16] = { 0 };
            const auto& colorArray = i.GetArray();

            Color color;
            color.r = colorArray[0].GetInt();
            color.g = colorArray[1].GetInt();
            color.b = colorArray[2].GetInt();
            color.a = colorArray[3].GetInt();
            list.PushBack(color);
        }
    }
    break;
    }

    return true;
}

bool TypeReader::ReadTypeListValue(TypeListValue& listValue, rapidjson::Value& jsonValue)
{
    if (!jsonValue.HasMember(TypeName) || !jsonValue[TypeName].IsString())
        return false;

    TypeInfo* type = TypeDatabase::Get().FindType(jsonValue[TypeName].GetString());
    if (!type)
        return false;

    if (jsonValue.HasMember(ValueName) && jsonValue[ValueName].IsArray())
    {
        auto& fieldValue = jsonValue[ValueName];

        const auto& array = fieldValue.GetArray();

        for (auto& i : array)
        {
            auto itemValue = listValue.PushBack(type);
            ReadTypeValue(itemValue, i);
        }
    }

    return true;
}

bool TypeReader::ReadTypeValue(TypeValue* destinationValue, rapidjson::Value& jsonValue)
{
    if (!jsonValue.HasMember(TypeName) || !jsonValue[TypeName].IsString())
        return false;

    TypeInfo* type = TypeDatabase::Get().FindType(jsonValue[TypeName].GetString());
    if (!type)
        return false;

    destinationValue->SetType(type);

    if (jsonValue.HasMember(Fields) && jsonValue[Fields].IsObject())
    {
        for (auto& field : jsonValue[Fields].GetObject())
        {
            int index = type->FindFieldIndex(field.name.GetString());
            if (index >= 0)
            {
                auto fieldInfo = type->GetField(index);
                switch (fieldInfo->GetType())
                {
                case FieldType::Primitive:
                    AddPrimtiveField(destinationValue, index, static_cast<PrimitiveFieldInfo*>(fieldInfo), field.value);
                    break;

                case FieldType::PrimitiveList:
                    AddPrimtiveListField(destinationValue, index, static_cast<PrimitiveFieldInfo*>(fieldInfo), field.value);
                    break;

                case FieldType::Type:
                    ReadTypeValue(destinationValue->GetTypeFieldValue(index), field.value);
                    break;

                case FieldType::TypeList:
                    ReadTypeListValue(destinationValue->GetTypeListFieldValue(index), field.value);
                    break;
                }
            }
        }
    }

    return true;
}

bool TypeReader::Read(TypeValue* value, const std::string& fileName)
{
    std::string fileData = ReadFileText(fileName.c_str());
    if (fileData.empty())
        return false;

    Document d;
    d.Parse(fileData.c_str());

    RootDocument = &d;

    if (RootDocument->HasMember(Version))
    {
        // (*RootDocument)[Version].GetString()
    }

    if (RootDocument->HasMember(RootData))
    {
        Value& rootObject = (*RootDocument)[RootData];
        if (!rootObject.IsObject())
            return false;

        return ReadTypeValue(value, rootObject);
    }
    return false;
}
