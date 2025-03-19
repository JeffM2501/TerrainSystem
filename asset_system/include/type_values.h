#pragma once

#include "type_database.h"

#include <map>
#include <vector>
#include<iterator>

namespace Types
{
    class FieldValue
    {
    public:
        virtual ~FieldValue() = default;

        template<typename T>
        inline T* GetAs()
        {
            return reinterpret_cast<T*>(this);
        }
    };


    class EnumerationFieldValue : public FieldValue
    {
    protected:
        int32_t Value;

    public:
        virtual ~EnumerationFieldValue() = default;
        int32_t GetValue() const { return Value; }

        template<class T>
        T GetValueAs() const { return T(Value); }

        void SetValue(const int32_t& newValue) { Value = newValue; }

        template<class T>
        void SetValueAs(const T& newValue) { Value = int32_t(newValue); }
    };


    template<typename T>
    class PrimitiveFieldValue : public FieldValue
    {
    protected:
        T Value;

    public:
        virtual ~PrimitiveFieldValue() = default;
        const T& GetValue() const { return Value; }
        void SetValue(const T& newValue) { Value = newValue; }
    };

    namespace PrimitiveFieldFactory
    {
        static std::unique_ptr<FieldValue> Create(PrimitiveType primitiveType);
    }

    class ListFieldValue : public FieldValue
    {
    public:
        virtual ~ListFieldValue() = default;

        virtual void Clear() = 0;

        virtual bool IsEmpty() const = 0;

        virtual size_t Size() const = 0;

        // todo, make this take the type for it's default value
        virtual size_t Add() = 0;

        virtual void Delete(size_t index) = 0;

        static std::unique_ptr<ListFieldValue> Create(PrimitiveType primitiveType);
    };

    template<typename T>
    class PrimitiveListFieldValue : public ListFieldValue
    {
    protected:
        std::vector<T> Values;

    public:
        virtual ~PrimitiveListFieldValue() = default;
        PrimitiveListFieldValue() = default;
        PrimitiveListFieldValue(const PrimitiveListFieldValue&) = delete;
        PrimitiveListFieldValue& operator = (const PrimitiveListFieldValue&) = delete;

        std::vector<T>& GetValues() { return Values; }
        const std::vector<T>& GetValues() const { return Values; }
        void SetValues(const std::vector<T>& newValues) { Values = newValues; }

        const T& GetValue(size_t index = 0) const { return Values[index]; }
        void SetValue(const T& newValue, size_t index = 0) { Values[index] = newValue; }

        typename std::vector<T>::iterator begin() { return Values.begin(); }
        typename std::vector<T>::const_iterator begin() const { return Values.cbegin(); }

        typename std::vector<T>::iterator end() { return Values.end(); }
        typename std::vector<T>::const_iterator end() const { return Values.cend(); }

        T& operator[] (int index) { return Values[index]; }

        typename std::vector<T>::iterator Erase(typename std::vector<T>::iterator at) { return Values.erase(at); }

        inline void Clear() override { Values.clear(); }

        inline bool IsEmpty() const override { return Values.empty(); }

        inline size_t Size() const override  { return Values.size(); }

        inline size_t Add() override
        {
            Values.resize(Values.size() + 1);
            return Values.size() - 1;
        }

        inline void Delete(size_t index) override
        {
            if (index < Values.size())
                Values.erase(Values.begin() + index);
        }

        void PushBack(const T& value) { return Values.push_back(value); }
    };

    class TypeListValue;

    using TypeValueFieldMap = std::map<int, std::unique_ptr<FieldValue>>;
    class TypeValue : public FieldValue
    {
    protected:
        const TypeInfo* Type = nullptr;

        TypeValueFieldMap Values;
    public:

        using Ptr = std::unique_ptr<TypeValue>;

        TypeValue() = default;
        TypeValue(const TypeInfo* t) { SetType(t); }

        void SetType(const TypeInfo* type);

        const TypeInfo* GetType() const { return Type; }
        const TypeValueFieldMap& GetTypeFieldValues() const { return Values; }

        bool FieldIsDefault(int fieldIndex) const
        {
            return Values.find(fieldIndex) == Values.end();
        }

        template<typename T>
        inline const T& GetFieldPrimitiveValue(int fieldIndex)
        {
            auto itr = Values.find(fieldIndex);
            if (itr != Values.end())
            {
                PrimitiveFieldValue<T>* valueItr = reinterpret_cast<PrimitiveFieldValue<T>*>(itr->second.get());
                return valueItr->GetValue();
            }

            const PrimitiveTypeFieldInfo<T>* fieldPtr = Type->GetField<PrimitiveTypeFieldInfo<T>>(fieldIndex);
            return fieldPtr->GetDefaultValue();
        }

        template<typename T>
        inline void SetFieldPrimitiveValue(int fieldIndex, const T& value)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
                itr = Values.insert_or_assign(fieldIndex, std::move(std::make_unique<PrimitiveFieldValue<T>>())).first;

            PrimitiveFieldValue<T>* valueItr = reinterpret_cast<PrimitiveFieldValue<T>*>(itr->second.get());
            valueItr->SetValue(value);
        }

        template<typename T>
        inline T GetFieldEnumerationValue(int fieldIndex)
        {
            auto itr = Values.find(fieldIndex);
            if (itr != Values.end())
            {
                EnumerationFieldValue* valueItr = reinterpret_cast<EnumerationFieldValue*>(itr->second.get());
                return valueItr->GetValueAs<T>();
            }

            const EnumerationFieldInfo* fieldPtr = Type->GetField<EnumerationFieldInfo>(fieldIndex);
            return T(FieldPtr->DefaultValue());
        }

        template<typename T>
        inline void SetFieldEnumerationValue(int fieldIndex, const T& value)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
                itr = Values.insert_or_assign(fieldIndex, std::move(std::make_unique<EnumerationFieldValue>())).first;

            EnumerationFieldValue* valueItr = reinterpret_cast<EnumerationFieldValue*>(itr->second.get());
            valueItr->SetValueAs(value);
        }

        void ResetFieldToDefault(int fieldIndex);

        TypeValue* GetTypeFieldValue(int fieldIndex);

        TypeValue* SetTypeFieldPointer(const TypeInfo* type, int fieldIndex);
        TypeValue* SetTypeFieldPointer(const std::string& typeName, int fieldIndex);

        template<typename T>
        PrimitiveListFieldValue<T>& GetPrimitiveListFieldValue(int fieldIndex)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
            {
                const PrimitiveFieldInfo* fieldPtr = Type->GetField<PrimitiveFieldInfo>(fieldIndex);

                auto value = ListFieldValue::Create(fieldPtr->GetPrimitiveType());
                itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
            }

            return *(PrimitiveListFieldValue<T>*)itr->second.get();
        }

        ListFieldValue& GetPrimitiveListFieldValue(int fieldIndex)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
            {
                const PrimitiveFieldInfo* fieldPtr = Type->GetField<PrimitiveFieldInfo>(fieldIndex);

                auto value = ListFieldValue::Create(fieldPtr->GetPrimitiveType());
                itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
            }

            return *(ListFieldValue*)itr->second.get();
        }

        TypeListValue& GetTypeListFieldValue(int fieldIndex);

        bool IsDefault() const { return Values.empty(); }

        template<class T>
        bool FieldHasAttribute(int fieldIndex) const
        {
            const auto* fieldInfo = Type->GetField(index);

            if (!fieldInfo)
                return false;

            if (fieldInfo->HasAttribute<T>())
                return true;

            if (fieldInfo->GetType() == FieldType::Type || fieldInfo->GetType() == FieldType::TypeList)
            {
                const TypeFieldInfo* typeFieldInfo = static_cast<const TypeFieldInfo*>(fieldInfo);
                return typeFieldInfo->TypePtr->HasAttribute<T>();
            }

            return false;
        }

        template<class T>
        const T* GetFieldAttribute(int fieldIndex) const
        {
            const FieldInfo* fieldInfo = Type->GetField(fieldIndex);

            if (!fieldInfo)
                return nullptr;

            if (fieldInfo->HasAttribute<T>())
                return fieldInfo->GetAttribute<T>();

            if (fieldInfo->GetType() == FieldType::Type || fieldInfo->GetType() == FieldType::TypeList)
            {
                const TypeFieldInfo* typeFieldInfo = static_cast<const TypeFieldInfo*>(fieldInfo);
                return typeFieldInfo->TypePtr->GetAttribute<T>();
            }

            return nullptr;
        }
    };

    using TypeValueList = std::vector<TypeValue::Ptr>;

    class TypeListValue : public FieldValue
    {
    protected:
        const TypeInfo* Type = nullptr;
        TypeValueList Values;
    public:
        TypeListValue() = default;
        TypeListValue(const TypeInfo* t) { Type = t; }

        virtual ~TypeListValue() = default;

        const TypeInfo* GetType() const { return Type; }

        TypeValueList& GetValues() { return Values; }
        const TypeValueList& GetValues() const { return Values; }

        typename std::vector<TypeValue::Ptr>::iterator begin() { return Values.begin(); }
        typename std::vector<TypeValue::Ptr>::const_iterator begin() const { return Values.cbegin(); }

        typename std::vector<TypeValue::Ptr>::iterator end() { return Values.end(); }
        typename std::vector<TypeValue::Ptr>::const_iterator end() const { return Values.cend(); }

        TypeValue& operator[] (size_t index) { return *Values[index].get(); }

        const TypeValue& operator[] (size_t index) const { return *Values[index].get(); }

        const TypeValue& Get (size_t index) const { return *Values[index].get(); }

        typename std::vector<TypeValue::Ptr>::iterator Erase(typename std::vector<TypeValue::Ptr>::iterator at) { return Values.erase(at); }

        void Clear() { Values.clear(); }

        bool IsEmpty() const { return Values.empty(); }

        size_t Size() const { return Values.size(); }

        TypeValue* PushBack(const TypeInfo* type) 
        {
            auto value = std::make_unique<TypeValue>(type);
            TypeValue* ret = value.get();
            Values.emplace_back(std::move(value));
            return ret;
        }

        TypeValue* PushBack(const std::string& typeName)
        {
            return PushBack(Type->TypeDB.FindType(typeName));
        }
    };

    class PrimitiveValueAccessor
    {
    public:
        PrimitiveValueAccessor(TypeValue* typeValue, int fieldIdex)
            : ParentValue(typeValue), Index(fieldIdex) {}

        PrimitiveValueAccessor(class ListFieldValue* listValue, int listIndex)
            : ListValue(listValue), Index(listIndex) {}

        bool IsDefaultValue() const
        {
            if (ListValue)
                return false;

            return ParentValue->FieldIsDefault(Index);
        }

        template<typename T>
        const T& GetValue() const
        {
            if (ListValue)
            {
                PrimitiveListFieldValue<T>* list = ListValue->GetAs<PrimitiveListFieldValue<T>>();
                return list->GetValue(Index);
            }
            return ParentValue->GetFieldPrimitiveValue<T>(Index);
        }

        template<typename T>
        void SetValue(const T& newValue)
        {
            if (ListValue)
                ListValue->GetAs<PrimitiveListFieldValue<T>>()->SetValue(newValue, Index);
            else
                ParentValue->SetFieldPrimitiveValue<T>(Index, newValue);
        }

        void SetIndex(int index) { Index = index; }

    protected:
        TypeValue* ParentValue = nullptr;
        class ListFieldValue* ListValue = nullptr;

        int Index = 0;
    };

}
