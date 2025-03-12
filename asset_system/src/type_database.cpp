#include "type_database.h"

using namespace Types;

//--------------------------------------------------------------
//   TypeInfo
//--------------------------------------------------------------
int TypeInfo::GetLocalFieldIndex(int index) const
{
    if (ParentType)
        return index - ParentType->GetFieldCount();

    return index;
}

FieldInfo* TypeInfo::GetField(int index)
{
    int localIndex = GetLocalFieldIndex(index);
    if (localIndex < 0)
    {
        if (ParentType != nullptr)
            return ParentType->GetField(index);

        return nullptr;
    }

    return Fields[localIndex].get();
}

const FieldInfo* TypeInfo::GetField(int index) const
{
    int localIndex = GetLocalFieldIndex(index);
    if (localIndex < 0)
    {
        if (ParentType != nullptr)
            return ParentType->GetField(index);

        return nullptr;
    }

    return Fields[localIndex].get();
}


int TypeInfo::FindFieldIndex(const std::string& name)
{
    for (int i = 0; i < GetFieldCount(); i++)
    {
        FieldInfo* field = GetField(i);
        if (field && field->GetName() == name)
            return i;
    }
    return -1;
}

int TypeInfo::GetFieldCount() const
{
    int count = int(Fields.size());
    if (ParentType)
        count += ParentType->GetFieldCount();
    return count;
}

TypeInfo::FieldIterator TypeInfo::begin() const
{
    return FieldIterator(this);
}

TypeInfo::FieldIterator TypeInfo::end() const
{
    return FieldIterator(this, GetFieldCount());
}

const EnumerationFieldInfo* TypeInfo::AddEnumerationField(const std::string& name, const std::string& enumName, int32_t defaultValue)
{
    std::unique_ptr<EnumerationFieldInfo> field = std::make_unique<EnumerationFieldInfo>(name, TypeDB.FindEnumeratione(enumName), defaultValue);

    EnumerationFieldInfo* ptr = field.get();
    Fields.push_back(std::move(field));
    return ptr;
}

const TypeFieldInfo* TypeInfo::AddTypeField(const std::string& name, const std::string& typeName, bool isPointer, const std::string& ptrTypeName)
{
    std::unique_ptr<TypeFieldInfo> field = std::make_unique<TypeFieldInfo>(name, TypeDB.FindType(typeName), isPointer, TypeDB.FindType(ptrTypeName));

    TypeFieldInfo* ptr = field.get();
    Fields.push_back(std::move(field));
    return ptr;
}

const TypeListFieldInfo* TypeInfo::AddTypeListField(const std::string& name, const std::string& typeName, bool isPointer)
{
    std::unique_ptr<TypeListFieldInfo> field = std::make_unique<TypeListFieldInfo>(name, TypeDB.FindType(typeName), isPointer);

    TypeListFieldInfo* ptr = field.get();
    Fields.push_back(std::move(field));
    return ptr;
}

//--------------------------------------------------------------
//   TypeDatabase
//--------------------------------------------------------------
uint64_t TypeDatabase::GetTypeId(const std::string& typeName)
{
    return Hashes::CRC64Str(typeName);
}

TypeInfo* TypeDatabase::CreateType(const std::string& typeName)
{
    return CreateType(typeName, 0);
}

TypeInfo* TypeDatabase::CreateType(const std::string& typeName, const std::string& parentTypeName)
{
    return CreateType(typeName, GetTypeId(parentTypeName));
}

TypeInfo* TypeDatabase::CreateType(const std::string& typeName, size_t parentTypeID)
{
    size_t hash = GetTypeId(typeName);
    auto itr = Types.find(hash);
    if (itr != Types.end())
        return itr->second.get();

    std::unique_ptr<TypeInfo> type = std::make_unique<TypeInfo>(*this);
    type->TypeName = typeName;
    type->TypeId = hash;

    TypeInfo* ptr = type.get();

    auto parentTypeItr = Types.find(parentTypeID);
    if (parentTypeItr != Types.end())
        type->ParentType = parentTypeItr->second.get();

    Types.try_emplace(hash, std::move(type));

    return ptr;
}

TypeInfo* TypeDatabase::FindType(const std::string& typeName)
{
    return FindType(GetTypeId(typeName));
}

TypeInfo* TypeDatabase::FindType(size_t typeId)
{
    auto itr = Types.find(typeId);
    if (itr != Types.end())
        return itr->second.get();
    return nullptr;
}

EnumerationInfo* TypeDatabase::CreateEnumeration(const std::string& typeName)
{
    size_t hash = GetTypeId(typeName);
    auto itr = Enumerations.find(hash);
    if (itr != Enumerations.end())
        return itr->second.get();

    std::unique_ptr<EnumerationInfo> enumeratrion = std::make_unique<EnumerationInfo>();
    enumeratrion->TypeName = typeName;
    enumeratrion->TypeId = hash;

    EnumerationInfo* ptr = enumeratrion.get();

    Enumerations.try_emplace(hash, std::move(enumeratrion));

    return ptr;
}

EnumerationInfo* TypeDatabase::FindEnumeratione(const std::string& typeName)
{
    return FindEnumeratione(GetTypeId(typeName));
}

EnumerationInfo* TypeDatabase::FindEnumeratione(size_t typeId)
{
    auto itr = Enumerations.find(typeId);
    if (itr != Enumerations.end())
        return itr->second.get();
    return nullptr;
}

bool TypeDatabase::IsBaseClassOf(const std::string testType, const std::string& possibleBase)
{
    return IsBaseClassOf(GetTypeId(testType), GetTypeId(possibleBase));
}

bool TypeDatabase::IsBaseClassOf(size_t testType, size_t possibleBase)
{
    TypeInfo* testTypePtr = FindType(testType);
    TypeInfo* possibleBasePtr = FindType(possibleBase);

    if (!testTypePtr || !possibleBasePtr)
        return false;

    while (testTypePtr != nullptr)
    {
        if (possibleBasePtr == testTypePtr)
            return true;
        testTypePtr = testTypePtr->ParentType;
    }

    return false;
}