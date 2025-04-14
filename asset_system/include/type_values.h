#pragma once

#include "type_database.h"
#include "field_path.h"
#include "Events.h"
#include <map>
#include <vector>
#include <iterator>

namespace Types
{
    class TypeValue;

    class ValueChangedRecord
    {
    public:
        ValueChangedRecord() = default;
        ValueChangedRecord(const ValueChangedRecord&) = delete;
        ValueChangedRecord& operator = (const ValueChangedRecord&) = delete;

        virtual ~ValueChangedRecord() = default;

        using Ptr = std::shared_ptr<ValueChangedRecord>;

    };

    class ValueChangedEvent
    {
    public:
        TypeValue* Value = nullptr;
        FieldPath Path;

        ValueChangedRecord::Ptr Record = nullptr;

        enum ValueRecordType
        {
            Primitive,
            Enumeration
        };

        ValueRecordType RecordType = ValueRecordType::Primitive;

        template<class T>
        T* GetRecordAs()
        {
            return static_cast<T*>(Record.get());
        }
    };

    template<class T>
    class PrimitiveValueChangedRecord : public ValueChangedRecord
    {
    public:
        T OldValue;
        T NewValue;

        using Ptr = std::shared_ptr<PrimitiveValueChangedRecord>;
    };

    class EnumValueChangedRecord : public ValueChangedRecord
    {
    public:
        int OldValue;
        int NewValue;

        using Ptr = std::shared_ptr<EnumValueChangedRecord>;
    };

    class FieldValue
    {
    protected:
        TypeValue* ParentValue = nullptr;
        FieldPath SubPath;

    public:
        FieldValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath())
            : ParentValue(parentValue)
            , SubPath(path)
        {
        }
        virtual ~FieldValue() = default;

        template<typename T>
        inline T* GetAs()
        {
            return reinterpret_cast<T*>(this);
        }

        virtual TypeValue* GetParent() { return ParentValue; }
        virtual const TypeValue* GetParent() const { return ParentValue; }
    };

    class EnumerationFieldValue : public FieldValue
    {
    protected:
        int32_t Value = 0;

    public:
        EnumerationFieldValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) {}
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
        PrimitiveFieldValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) {}
        virtual ~PrimitiveFieldValue() = default;
        const T& GetValue() const { return Value; }
        void SetValue(const T& newValue) { Value = newValue; }
    };

    class ListFieldValue : public FieldValue
    {
    public:
        ListFieldValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) {}
        virtual ~ListFieldValue() = default;

        virtual void Clear() = 0;

        virtual bool IsEmpty() const = 0;

        virtual size_t Size() const = 0;

        // todo, make this take the type for it's default value
        virtual size_t Add() = 0;

        virtual void Delete(size_t index) = 0;

        static std::unique_ptr<ListFieldValue> Create(PrimitiveType primitiveType, TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath());
    };

    template<typename T>
    class PrimitiveListFieldValue : public ListFieldValue
    {
    protected:
        std::vector<T> Values;

    public:
        PrimitiveListFieldValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : ListFieldValue(parentValue, path) {}
        virtual ~PrimitiveListFieldValue() = default;
        PrimitiveListFieldValue() = default;
        PrimitiveListFieldValue(const PrimitiveListFieldValue&) = delete;
        PrimitiveListFieldValue& operator = (const PrimitiveListFieldValue&) = delete;

        Events::EventSource<ValueChangedEvent> OnValueChanged;

        void CallValueChanged(ValueChangedEvent& eventRecord)
        {
            eventRecord.Value = this;
            OnValueChanged.Invoke(eventRecord);

            if (ParentValue)
            {
                eventRecord.Path.PushBack(path);
                ParentValue->CallValueChanged(eventRecord);
            }
        }

        std::vector<T>& GetValues() { return Values; }
        const std::vector<T>& GetValues() const { return Values; }
        void SetValues(const std::vector<T>& newValues) { Values = newValues; }

        const T& GetValue(size_t index = 0) const { return Values[index]; }
        bool SetValue(const T& newValue, size_t index = 0)
        {
            if (index == value.size())
            {
                PushBack(newValue);
                return true;
            }
            if (index >= Values.size())
                return false;

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath + FieldPath::Index(index);
            eventRecord.Record = std::make_shared<PrimitiveValueChangedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->OldValue = Values[index];
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->NewValue = newValue;
            Values[index] = newValue;
            CallPrimitiveValueChanged(eventRecord);
            return true;
        }

        typename std::vector<T>::iterator begin() { return Values.begin(); }
        typename std::vector<T>::const_iterator begin() const { return Values.cbegin(); }

        typename std::vector<T>::iterator end() { return Values.end(); }
        typename std::vector<T>::const_iterator end() const { return Values.cend(); }

        T& operator[] (int index) { return Values[index]; }

        typename std::vector<T>::iterator Erase(typename std::vector<T>::iterator at) { return Values.erase(at); }

        inline void Clear() override { Values.clear(); }

        inline bool IsEmpty() const override { return Values.empty(); }

        inline size_t Size() const override { return Values.size(); }

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
        size_t ID = 0;
        void* UserData = nullptr;

        using Ptr = std::unique_ptr<TypeValue>;

        TypeValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) {}
        TypeValue(const TypeInfo* t, TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) { SetType(t); }

        Events::EventSource<ValueChangedEvent> OnValueChanged;

        TypeValue* GetParent() override
        {
            if (ParentValue)
                return ParentValue->GetParent();

            return reinterpret_cast<TypeValue*>(this);
        }

        const TypeValue* GetParent() const override
        {
            if (ParentValue)
                return ParentValue->GetParent();

            return reinterpret_cast<const TypeValue*>(this);
        }

        void CallValueChanged(ValueChangedEvent& eventRecord)
        {
            eventRecord.Value = this;
            OnValueChanged.Invoke(eventRecord);

            if (ParentValue)
            {
                eventRecord.Path.PushFront(SubPath);
                ParentValue->CallValueChanged(eventRecord);
            }
        }

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
            {
                FieldPath path(FieldPath::Field(fieldIndex));
                itr = Values.insert_or_assign(fieldIndex, std::move(std::make_unique<PrimitiveFieldValue<T>>(this, path))).first;
            }

            PrimitiveFieldValue<T>* valueItr = reinterpret_cast<PrimitiveFieldValue<T>*>(itr->second.get());

            ValueChangedEvent eventRecord;
            eventRecord.Record = std::make_shared<PrimitiveValueChangedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->OldValue = valueItr->GetValue();
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->NewValue = value;
            valueItr->SetValue(value);

            eventRecord.Path.Elements.push_back(FieldPath::Field(fieldIndex));
            CallValueChanged(eventRecord);
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
            return T(fieldPtr->DefaultValue);
        }

        template<typename T>
        inline void SetFieldEnumerationValue(int fieldIndex, const T& value)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
                itr = Values.insert_or_assign(fieldIndex, std::move(std::make_unique<EnumerationFieldValue>(this, SubPath + FieldPath::Field(fieldIndex)))).first;

            EnumerationFieldValue* valueItr = reinterpret_cast<EnumerationFieldValue*>(itr->second.get());

            ValueChangedEvent eventRecord;
            eventRecord.Record = std::make_shared<EnumValueChangedRecord>();
            eventRecord.GetRecordAs<EnumValueChangedRecord>()->OldValue = valueItr->GetValue();
            valueItr->SetValueAs(value);
            eventRecord.GetRecordAs<EnumValueChangedRecord>()->NewValue = valueItr->GetValue();
            CallValueChanged(eventRecord);
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

                auto value = ListFieldValue::Create(fieldPtr->GetPrimitiveType(), this, SubPath + FieldPath::Field(fieldIndex));
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

                auto value = ListFieldValue::Create(fieldPtr->GetPrimitiveType(), this, SubPath + FieldPath::Field(fieldIndex));
                itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
            }

            return *(ListFieldValue*)itr->second.get();
        }

        template<typename T>
        void PushBackPrimitiveListFieldValue(int fieldIndex, T value)
        {
            auto itr = Values.find(fieldIndex);
            if (itr == Values.end())
            {
                const PrimitiveFieldInfo* fieldPtr = Type->GetField<PrimitiveFieldInfo>(fieldIndex);

                auto value = ListFieldValue::Create(fieldPtr->GetPrimitiveType(), this, SubPath + FieldPath::Field(fieldIndex));
                itr = Values.insert_or_assign(fieldIndex, std::move(value)).first;
            }

            (PrimitiveListFieldValue<T>*)itr->second->push_back(value);

            // TODO, call add event
        }

        TypeListValue& GetTypeListFieldValue(int fieldIndex);

        bool IsDefault() const { return Values.empty(); }

        template<class T>
        void SetPrimtiveFieldFromPath(const FieldPath& path, const T& value, int pathIndex = 0)
        {
            if (path.Elements[pathIndex].Type == FieldPath::ElementType::Field)
            {
                int fieldIndex = path.Elements[pathIndex].Index;

                auto* fieldTypeInfo = GetType()->GetField(fieldIndex);

                if (fieldTypeInfo->IsPrimtive())
                {
                    if (fieldTypeInfo->IsList())
                    {
                        GetPrimitiveListFieldValue<T>(fieldIndex).SetValue(value, path.Elements[pathIndex + 1].Index);
                    }
                    else
                    {
                        SetFieldPrimitiveValue(fieldIndex, value);
                    }
                }
                else if (fieldTypeInfo->IsType())
                {
                    if (fieldTypeInfo->IsList())
                    {
                        auto& list = GetTypeListFieldValue(fieldIndex);

                        pathIndex++;
                        int listIndex = path.Elements[pathIndex].Index;

                        if (listIndex == list.Size())
                        {
                            list.PushBack(fieldTypeInfo->GetType())->SetPrimtiveFieldFromPath(path, value, pathIndex + 1);
                        }
                        else
                        {
                            list[listIndex].SetPrimtiveFieldFromPath(path, value, pathIndex + 1); W
                        }
                    }
                    else
                    {
                        GetTypeFieldValue(fieldIndex)->SetPrimtiveFieldFromPath(path, value, pathIndex + 1);
                    }
                }
            }
        }

        void SetEnumFieldFromPath(const FieldPath& path, const int& value, int pathIndex = 0)
        {
            if (path.Elements[pathIndex].Type == FieldPath::ElementType::Field)
            {
                int fieldIndex = path.Elements[pathIndex].Index;

                auto* fieldTypeInfo = GetType()->GetField(fieldIndex);

                if (fieldTypeInfo->IsEnum())
                {
                    if (!fieldTypeInfo->IsList())
                    {
                        SetFieldEnumerationValue(fieldIndex, value);
                    }
                }
                else if (fieldTypeInfo->IsType())
                {
                    if (!fieldTypeInfo->IsList())
                    {
                        GetTypeFieldValue(fieldIndex)->SetEnumFieldFromPath(path, value, pathIndex + 1);
                    }
                }
            }
        }

        std::string_view GetFieldNameFromPath(const FieldPath& path, int pathIndex = 0);
    };

    using TypeValueList = std::vector<TypeValue::Ptr>;

    class TypeListValue : public FieldValue
    {
    protected:
        const TypeInfo* Type = nullptr;
        TypeValueList Values;
    public:
        TypeListValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) {}
        TypeListValue(const TypeInfo* t, TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : FieldValue(parentValue, path) { Type = t; }

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

        const TypeValue& Get(size_t index) const { return *Values[index].get(); }

        typename std::vector<TypeValue::Ptr>::iterator Erase(typename std::vector<TypeValue::Ptr>::iterator at) { return Values.erase(at); }

        void Clear() { Values.clear(); }

        bool IsEmpty() const { return Values.empty(); }

        size_t Size() const { return Values.size(); }

        TypeValue* PushBack(const TypeInfo* type)
        {
            FieldPath childPath = SubPath + FieldPath::Index(int(Values.size()) - 1);
            auto value = std::make_unique<TypeValue>(type, ParentValue, childPath);
            TypeValue* ret = value.get();
            Values.emplace_back(std::move(value));
            return ret;
        }

        TypeValue* PushBack(const std::string& typeName)
        {
            return PushBack(TypeDatabase::Get().FindType(typeName));
        }
    };
}
