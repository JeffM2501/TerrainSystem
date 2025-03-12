#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>

namespace Types
{
    enum class FieldType
    {
        Enumeration,
        Primitive,
        Type,
        PrimitiveList,
        TypeList,
    };

    class FieldInfo
    {
    protected:
        FieldType Type = FieldType::Type;
        int Index = 0;
        std::string Name;

        // todo, attributes

    public:
        FieldInfo(const std::string& name)
            : Name(name)
        {}

        virtual ~FieldInfo() = default;

        inline FieldType GetType() const { return Type; }
        inline const std::string& GetName() const { return Name; }
    };

    class EnumerationFieldInfo : public FieldInfo
    {
    public:
        class EnumerationInfo* TypePtr = nullptr;

        int32_t DefaultValue = 0;

        EnumerationFieldInfo(const std::string& name, class EnumerationInfo* type, int32_t defaultValue = 0)
            : FieldInfo(name)
            , TypePtr(type)
            , DefaultValue(defaultValue)
        {
            Type = FieldType::Enumeration;
        }
    };

    class TypeFieldInfo : public FieldInfo
    {
    public:
        TypeFieldInfo(const std::string& name, class TypeInfo* type, bool isPointer = false, TypeInfo* defaultPtr = nullptr)
            : FieldInfo(name)
            , TypePtr(type)
            , IsPointer(isPointer)
            , DefaultPtrType(defaultPtr)
        {
            Type = FieldType::Type;
        }

        class TypeInfo* TypePtr = nullptr;

        class TypeInfo* DefaultPtrType = nullptr;

        bool IsPointer = false;
    };

    class TypeListFieldInfo : public FieldInfo
    {
    public:
        TypeListFieldInfo(const std::string& name, class TypeInfo* type, bool isPointer = false)
            : FieldInfo(name)
            , TypePtr(type)
        {
            Type = FieldType::TypeList;
        }

        class TypeInfo* TypePtr = nullptr;

        bool IsPointer = false;
    };

    enum class PrimitiveType
    {
        Unknown,
        Bool,
        Char,
        UInt8,
        UInt16,
        Int16,
        UInt32,
        Int32,
        UInt64,
        Int64,
        Float32,
        Double64,
        String,
        Vector2,
        Vector3,
        Vector4,
        Rectangle,
        Matrix, 
        GUID
    };

    class PrimitiveFieldInfo : public FieldInfo
    {
    protected:
        PrimitiveType PrimType = PrimitiveType::Unknown;

    public:
        PrimitiveFieldInfo(const std::string& name, PrimitiveType primType, bool isList)
            : FieldInfo(name)
            , PrimType(primType)
        {
            Type = isList ? FieldType::PrimitiveList : FieldType::Primitive;
        }

        inline PrimitiveType GetPrimitiveType() const { return PrimType; }

        virtual ~PrimitiveFieldInfo() = default;
    };

    template<typename T>
    class PrimitiveTypeFieldInfo : public PrimitiveFieldInfo
    {
    protected:
        T DefaultValue;
    public:
        PrimitiveTypeFieldInfo(const std::string& name, const T& defaultValue, PrimitiveType primType)
            : PrimitiveFieldInfo(name, primType, false)
            , DefaultValue(defaultValue)
        {
            Type = FieldType::Primitive;
        }

        inline const T& GetDefaultValue() const { return DefaultValue; }

        virtual ~PrimitiveTypeFieldInfo() = default;
    };
}