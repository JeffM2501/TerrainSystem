#pragma once

#include "CRC64.h"

#include <string>

namespace AttributeTypes
{
#define DEFINE_ATTRIBUTE(T) \
	static uint64_t AttributeId() {return Hashes::CRC64Str(#T);}\
	uint64_t GetAttributeId() override {return Hashes::CRC64Str(#T);}

    class Attribute
    {
    public:
        virtual uint64_t GetAttributeId() = 0;
    };

    class ReadOnlyAttribute : public Attribute
    {
    public:
        DEFINE_ATTRIBUTE(ReadOnlyAttribute)
    };

    class HiddenAttribute : public Attribute
    {
    public:
        DEFINE_ATTRIBUTE(HiddenAttribute)
    };

    class NoSerializationAttribute : public Attribute
    {
    public:
        DEFINE_ATTRIBUTE(NoSerializationAttribute)
    };


    class CustomEditorAttribute : public Attribute
    {
    public:
        DEFINE_ATTRIBUTE(CustomEditorAttribute);

        CustomEditorAttribute(std::string_view editorName) : EditorName(editorName) {}

        std::string EditorName;
    };

    class DisplayNameAttribute : public Attribute
    {
    public:
        DEFINE_ATTRIBUTE(DisplayNameAttribute);

        DisplayNameAttribute(std::string_view name) : Name(name) {}

        std::string Name;
    };

    class AttributeContainer
    {
    protected:
        std::unordered_map<uint64_t, std::unique_ptr<AttributeTypes::Attribute>> Attributes;

    public:
        template <class T, typename ...Args>
        T* AddAttribute(Args&&... args)
        {
            auto attrib = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = attrib.get();

            Attributes.try_emplace(T::AttributeId(), std::move(attrib));
            return ptr;
        }

        template<class T>
        bool HasAttribute() const
        {
            return Attributes.find(T::AttributeId()) != Attributes.end();
        }

        template<class T>
        const T* GetAttribute() const
        {
            auto itr = Attributes.find(T::AttributeId());
            if (itr == Attributes.end())
                return nullptr;

            return static_cast<T*>(itr->second.get());
        }
    };
}