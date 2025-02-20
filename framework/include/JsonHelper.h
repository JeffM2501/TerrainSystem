#pragma once

#include "rapidjson/document.h"

namespace JSONHelper
{
    template<class T>
    inline void SetOrCreateValue(rapidjson::Value& rootValue, const char* name, const T& value, rapidjson::Document& document)
    {
        if (!rootValue.HasMember(name))
        {
            rapidjson::Value nameValue(name, document.GetAllocator());
            rapidjson::Value newValue;
            newValue.Set(value);
            rootValue.AddMember(nameValue, std::move(newValue), document.GetAllocator());
        }
        else
        {
            auto valueItem = rootValue.FindMember(name);
            valueItem->value.Set(value);
        }
    }

    template<class T>
    inline T GetValue(rapidjson::Value& rootValue, const char* name, const T& defaultValue)
    {
        if (rootValue.HasMember(name))
        {
            auto valueItem = rootValue.FindMember(name);

            if (!valueItem->value.IsNull() && valueItem->value.Is<T>())
                return valueItem->value.Get<T>();
        }
        
        return defaultValue;
    }
}