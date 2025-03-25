#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "field_info.h"
#include "raylib.h"
#include "CRC64.h"
#include "GUID.h"
#include "attributes.h"

namespace Types
{
	class TypeInfo : public AttributeTypes::AttributeContainer
	{
	private:
        template<class T>
        inline PrimitiveTypeFieldInfo<T>* AddPrimitiveField(const std::string& name, const T& defaultValue, PrimitiveType primType)
        {
            std::unique_ptr<PrimitiveTypeFieldInfo<T>> field = std::make_unique<PrimitiveTypeFieldInfo<T>>(name, defaultValue, primType);

            PrimitiveTypeFieldInfo<T>* ptr = field.get();
            Fields.emplace_back(std::move(field));
            return ptr;
        }

		std::vector<std::unique_ptr<FieldInfo>> Fields;

		int GetLocalFieldIndex(int index) const;

	public:
		std::string TypeName;
		uint64_t TypeId = 0;
		TypeInfo* ParentType = nullptr;

		TypeInfo() {}

		FieldInfo* GetField(int index);
		const FieldInfo* GetField(int index) const;

		template<class T>
		inline T* GetField(int index)
		{
			return reinterpret_cast<T*>(GetField(index));
		}

		template<class T>
		inline const T* GetField(int index) const
		{
			return reinterpret_cast<const T*>(GetField(index));
		}

		int FindFieldIndex(const std::string& name);

		int GetFieldCount() const;

		struct FieldIterator
		{
			const TypeInfo* Type = nullptr;
			int Index = 0;

			FieldIterator(const TypeInfo* type, int index = 0) : Type(type), Index(index) {};

			inline const FieldInfo* operator * () { return Type->GetField(Index); }
			inline const FieldInfo* Get() { return Type->GetField(Index); }

			inline FieldIterator operator++(int)
			{
				FieldIterator newItr(Type);
				newItr.Index = Index + 1;
				return newItr;
			}

			inline FieldIterator& operator++()
			{
				Index++;
				return *this;
			}

			inline bool operator == (const FieldIterator& c)
			{
				return Index == c.Index && Type == c.Type;
			}

			inline bool operator != (const FieldIterator& c)
			{
				return Index != c.Index || Type != c.Type;
			}
		};

		FieldIterator begin() const;
		FieldIterator end() const;

        template<class T>
        inline PrimitiveTypeFieldInfo<T>* AddPrimitiveField(const std::string& name, const T& defaultValue)
        {
            return AddPrimitiveField<T>(name, defaultValue, PrimitiveType::Unknown);
        }

		EnumerationFieldInfo* AddEnumerationField(const std::string& name, const std::string& enumName, int32_t defaultValue = 0);

		TypeFieldInfo* AddTypeField(const std::string& name, const std::string& typeName, bool isPointer = false, const std::string& ptrTypeName = std::string());

        TypeListFieldInfo* AddTypeListField(const std::string& name, const std::string& typeName, bool isPointer = false);

		PrimitiveFieldInfo* AddPrimitiveListField(const std::string& name, PrimitiveType primType)
		{
			std::unique_ptr<PrimitiveFieldInfo> field = std::make_unique<PrimitiveFieldInfo>(name, primType, true);

			PrimitiveFieldInfo* ptr = field.get();
			Fields.emplace_back(std::move(field));
			return ptr;
		}

		template<class T>
		bool FieldHasAttribute(int fieldIndex) const
		{
			const auto* fieldInfo = GetField(fieldIndex);

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
			const FieldInfo* fieldInfo = GetField(fieldIndex);

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

    // specializations
    template<>
    inline PrimitiveTypeFieldInfo<bool>* TypeInfo::AddPrimitiveField(const std::string& name, const bool& defaultValue)
    {
        return AddPrimitiveField<bool>(name, defaultValue, PrimitiveType::Bool);
    }

    template<>
    inline PrimitiveTypeFieldInfo<char>* TypeInfo::AddPrimitiveField(const std::string& name, const char& defaultValue)
    {
        return AddPrimitiveField<char>(name, defaultValue, PrimitiveType::Char);
    }

    template<>
    inline PrimitiveTypeFieldInfo<uint8_t>* TypeInfo::AddPrimitiveField(const std::string& name, const uint8_t& defaultValue)
    {
        return AddPrimitiveField<uint8_t>(name, defaultValue, PrimitiveType::UInt8);
    }

    template<>
    inline  PrimitiveTypeFieldInfo<uint16_t>* TypeInfo::AddPrimitiveField(const std::string& name, const uint16_t& defaultValue)
    {
        return AddPrimitiveField<uint16_t>(name, defaultValue, PrimitiveType::UInt16);
    }

    template<>
    inline PrimitiveTypeFieldInfo<int16_t>* TypeInfo::AddPrimitiveField(const std::string& name, const int16_t& defaultValue)
    {
        return AddPrimitiveField<int16_t>(name, defaultValue, PrimitiveType::Int16);
    }

    template<>
    inline PrimitiveTypeFieldInfo<uint32_t>* TypeInfo::AddPrimitiveField(const std::string& name, const uint32_t& defaultValue)
    {
        return AddPrimitiveField<uint32_t>(name, defaultValue, PrimitiveType::UInt32);
    }

    template<>
    inline PrimitiveTypeFieldInfo<int32_t>* TypeInfo::AddPrimitiveField(const std::string& name, const int32_t& defaultValue)
    {
        return AddPrimitiveField<int32_t>(name, defaultValue, PrimitiveType::Int32);
    }

    template<>
    inline PrimitiveTypeFieldInfo<uint64_t>* TypeInfo::AddPrimitiveField(const std::string& name, const uint64_t& defaultValue)
    {
        return AddPrimitiveField<uint64_t>(name, defaultValue, PrimitiveType::UInt64);
    }

    template<>
    inline PrimitiveTypeFieldInfo<int64_t>* TypeInfo::AddPrimitiveField(const std::string& name, const int64_t& defaultValue)
    {
        return AddPrimitiveField<int64_t>(name, defaultValue, PrimitiveType::Int64);
    }

    template<>
    inline PrimitiveTypeFieldInfo<float>* TypeInfo::AddPrimitiveField(const std::string& name, const float& defaultValue)
    {
        return AddPrimitiveField<float>(name, defaultValue, PrimitiveType::Float32);
    }

    template<>
    inline PrimitiveTypeFieldInfo<double>* TypeInfo::AddPrimitiveField(const std::string& name, const double& defaultValue)
    {
        return AddPrimitiveField<double>(name, defaultValue, PrimitiveType::Double64);
    }

    template<>
    inline PrimitiveTypeFieldInfo<std::string>* TypeInfo::AddPrimitiveField(const std::string& name, const std::string& defaultValue)
    {
        return AddPrimitiveField<std::string>(name, defaultValue, PrimitiveType::String);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Vector2>* TypeInfo::AddPrimitiveField(const std::string& name, const Vector2& defaultValue)
    {
        return AddPrimitiveField<Vector2>(name, defaultValue, PrimitiveType::Vector2);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Vector3>* TypeInfo::AddPrimitiveField(const std::string& name, const Vector3& defaultValue)
    {
        return AddPrimitiveField<Vector3>(name, defaultValue, PrimitiveType::Vector3);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Vector4>* TypeInfo::AddPrimitiveField(const std::string& name, const Vector4& defaultValue)
    {
        return AddPrimitiveField<Vector4>(name, defaultValue, PrimitiveType::Vector4);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Rectangle>* TypeInfo::AddPrimitiveField(const std::string& name, const Rectangle& defaultValue)
    {
        return AddPrimitiveField<Rectangle>(name, defaultValue, PrimitiveType::Rectangle);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Matrix>* TypeInfo::AddPrimitiveField(const std::string& name, const Matrix& defaultValue)
    {
        return AddPrimitiveField<Matrix>(name, defaultValue, PrimitiveType::Matrix);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Hashes::GUID>* TypeInfo::AddPrimitiveField(const std::string& name, const Hashes::GUID& defaultValue)
    {
        return AddPrimitiveField<Hashes::GUID>(name, defaultValue, PrimitiveType::GUID);
    }

    template<>
    inline PrimitiveTypeFieldInfo<Color>* TypeInfo::AddPrimitiveField(const std::string& name, const Color& defaultValue)
    {
        return AddPrimitiveField<Color>(name, defaultValue, PrimitiveType::Color);
    }

	class EnumerationInfo
	{
	public:
		std::string TypeName;
		uint64_t TypeId = 0;

		std::map<int32_t, std::string> Values;
	};

	class TypeDatabase
	{
	private:
		std::unordered_map<uint64_t, std::unique_ptr<TypeInfo>> Types;
		std::unordered_map<uint64_t, std::unique_ptr<EnumerationInfo>> Enumerations;

	public:
		static TypeDatabase& Get();

		uint64_t GetTypeId(const std::string& typeName);

		TypeInfo* CreateType(const std::string& typeName);
		TypeInfo* CreateType(const std::string& typeName, const std::string& parentTypeName);
		TypeInfo* CreateType(const std::string& typeName, uint64_t parentTypeID);

		EnumerationInfo* CreateEnumeration(const std::string& typeName);

		TypeInfo* FindType(const std::string& typeName);
		TypeInfo* FindType(uint64_t typeId);

		EnumerationInfo* FindEnumeration(const std::string& typeName);
		EnumerationInfo* FindEnumeration(uint64_t typeId);

		bool IsBaseClassOf(const std::string testType, const std::string& possibleBase);
		bool IsBaseClassOf(uint64_t testType, uint64_t possibleBase);

		template <class T>
		inline T CreateTypeValue()
		{
			return T();
		}
	};
}
