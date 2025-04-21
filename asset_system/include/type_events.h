#pragma once

#include "type_database.h"
#include "field_path.h"

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

    template<class T>
    class PrimitiveListItemAddedRecord : public ValueChangedRecord
    {
    public:
        T NewValue;
        using Ptr = std::shared_ptr<PrimitiveListItemAddedRecord>;
    };

    template<class T>
    class PrimitiveListItemRemovedRecord : public ValueChangedRecord
    {
    public:
        T OldValue;
        using Ptr = std::shared_ptr<PrimitiveListItemRemovedRecord>;
    };

    template<class T>
    class PrimitiveListClearedRecord : public ValueChangedRecord
    {
    public:
        std::vector<T> OldValues;
        using Ptr = std::shared_ptr<PrimitiveListClearedRecord>;
    };

    class TypeListItemAddedRecord : public ValueChangedRecord
    {
    public:
        uint64_t NewTypeID;
        using Ptr = std::shared_ptr<TypeListItemAddedRecord>;
    };

    class TypeListItemRemovedRecord : public ValueChangedRecord
    {
    public:
        std::unique_ptr<TypeValue> OldValue;
        using Ptr = std::shared_ptr<TypeListItemRemovedRecord>;
    };

    class TypeListClearedRecord : public ValueChangedRecord
    {
    public:
        std::vector<std::unique_ptr<TypeValue>> OldValues;
        using Ptr = std::shared_ptr<TypeListClearedRecord>;
    };

    class ValueChangedEvent
    {
    public:
        TypeValue* Value = nullptr;
        FieldPath Path;

        ValueChangedRecord::Ptr Record = nullptr;

        enum ValueRecordType
        {
            PrimitiveChanged,
            EnumerationChanged,
            PrimitiveListItemAdded,
            PrimitiveListItemRemoved,
            PrimitiveListCleared,
            TypeListItemAdded,
            TypeListItemRemoved,
            TypeListCleared
        };

        ValueRecordType RecordType = ValueRecordType::PrimitiveChanged;

        template<class T>
        T* GetRecordAs()
        {
            return static_cast<T*>(Record.get());
        }
    };
}
