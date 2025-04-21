#pragma once

#include "type_database.h"
#include "type_events.h"
#include "type_field_value.h"
#include "field_path.h"
#include "Events.h"
#include <map>
#include <vector>
#include <iterator>

namespace Types
{
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
            eventRecord.Value = ParentValue;
            OnValueChanged.Invoke(eventRecord);

            if (ParentValue)
            {
                ParentValue->CallValueChanged(eventRecord);
            }
        }

        std::vector<T>& GetValues() { return Values; }
        const std::vector<T>& GetValues() const { return Values; }
        void SetValues(const std::vector<T>& newValues) { Values = newValues; }

        const T& GetValue(size_t index = 0) const
        {
            return Values[index];
        }

        bool SetValue(const T& newValue, size_t index = 0)
        {
            if (index == Values.size())
            {
                PushBack(newValue);
                return true;
            }
            if (index >= Values.size())
                return false;

            ValueChangedEvent eventRecord;
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::PrimitiveChanged;
            eventRecord.Path = SubPath + FieldPath::Index(int(index));
            eventRecord.Record = std::make_shared<PrimitiveValueChangedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->OldValue = Values[index];
            eventRecord.GetRecordAs<PrimitiveValueChangedRecord<T>>()->NewValue = newValue;
            Values[index] = newValue;
            CallValueChanged(eventRecord);
            return true;
        }

        typename std::vector<T>::iterator begin() { return Values.begin(); }
        typename std::vector<T>::const_iterator begin() const { return Values.cbegin(); }

        typename std::vector<T>::iterator end() { return Values.end(); }
        typename std::vector<T>::const_iterator end() const { return Values.cend(); }

        T& operator[] (int index) { return Values[index]; }

        typename std::vector<T>::iterator Erase(typename std::vector<T>::iterator at) { return Values.erase(at); }

        inline void Clear() override
        {
            Values.clear();

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath;
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::PrimitiveListCleared;
            eventRecord.Record = std::make_shared<PrimitiveListClearedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveListClearedRecord<T>>()->OldValues = Values;

            CallValueChanged(eventRecord);
        }

        inline bool IsEmpty() const override { return Values.empty(); }

        inline size_t Size() const override { return Values.size(); }

        inline size_t Add() override
        {
            Values.resize(Values.size() + 1);

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath + FieldPath::Index(int(Values.size()));
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::PrimitiveListItemAdded;
            eventRecord.Record = std::make_shared<PrimitiveListItemAddedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveListItemAddedRecord<T>>()->NewValue = Values[Values.size() - 1];

            CallValueChanged(eventRecord);

            return Values.size() - 1;
        }

        inline void Delete(size_t index) override
        {
            if (index >= Values.size())
                return;

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath + FieldPath::Index(int(index));
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::PrimitiveListItemRemoved;
            eventRecord.Record = std::make_shared<PrimitiveListItemRemovedRecord<T>>();
            eventRecord.GetRecordAs<PrimitiveListItemRemovedRecord<T>>()->OldValue = Values[index];
            Values.erase(Values.begin() + index);

            CallValueChanged(eventRecord);
        }

        void PushBack(const T& value) { return Values.push_back(value); }
    };
}
