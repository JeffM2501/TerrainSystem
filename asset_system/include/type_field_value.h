#pragma once

#include "type_database.h"
#include "type_events.h"
#include "field_path.h"
#include "Events.h"
#include <map>
#include <vector>
#include <iterator>

namespace Types
{
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
}