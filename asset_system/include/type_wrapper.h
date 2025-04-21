#pragma once


#include "type_database.h"
#include "type_values.h"
#include "type_io.h"

#include <memory>

namespace Types
{
	//----------------------------------------
	//        TypeWraper
	//----------------------------------------

	class TypeWraper
	{
	public:
		virtual~TypeWraper() = default;

		const TypeInfo* TypePtr = nullptr;
		std::unique_ptr<TypeValue> Value;

		TypeValue* ValuePtr = nullptr;

		template <class T>
		static std::unique_ptr<T> ReadAs(const std::string& fileName, bool createIfNotExist = true)
		{
			std::unique_ptr<TypeValue> value = std::make_unique<TypeValue>();
			TypeIO::TypeReader reader;

			if (!reader.Read(value.get(), fileName) || value->GetType()->TypeName != T::TypeName)
			{
				if (createIfNotExist)
				{
					return std::make_unique<T>();
				}

				return std::make_unique<T>(nullptr);
			}

			return std::make_unique<T>(std::move(value));
		}

		bool Write(const std::string& fileName)
		{
			TypeIO::TypeWriter writer;
			return writer.Write(ValuePtr, fileName);
		}

		inline bool IsValid() const { return ValuePtr != nullptr; }

	protected:
		TypeWraper(const std::string& typeName)
		{
			TypePtr = TypeDatabase::Get().FindType(typeName);
			Value = std::make_unique<TypeValue>(TypePtr);
			ValuePtr = Value.get();
		}

		TypeWraper(std::unique_ptr<TypeValue> value)
		{
			Value = std::move(value);
			ValuePtr = Value.get();
			TypePtr = ValuePtr->GetType();
		}

		TypeWraper(TypeValue* value)
		{
			TypePtr = value->GetType();
			ValuePtr = value;
		}

		inline int GetParentFieldCount() const
		{
			if (!TypePtr || !TypePtr->ParentType)
				return 0;

			return TypePtr->ParentType->GetFieldCount();
		}

		virtual void OnCreate() {}
	};

#define DEFINE_ENUM(T) \
public : \
    static constexpr char TypeName[] = #T;

#define DEFINE_TYPE(T) \
protected:  \
    T(const std::string& typeName): TypeWraper(typeName) {} \
public : \
    static constexpr char TypeName[] = #T;\
    T() : TypeWraper(T::TypeName) { OnCreate(); }\
    T(TypeValue* value) : TypeWraper(value) { OnCreate(); } \
    T(std::unique_ptr<TypeValue> value) : TypeWraper(std::move(value)) { OnCreate(); } \
    using Ptr = std::unique_ptr<T>;

#define DEFINE_DERIVED_TYPE(T, B) \
protected:  \
    T(const std::string& typeName): B(typeName) {} \
public : \
    static constexpr char TypeName[] = #T; \
    T() : B(T::TypeName) { OnCreate(); } \
    T(TypeValue* value) : B(value) { OnCreate(); } \
    T(std::unique_ptr<TypeValue> value) : B(std::move(value)) { OnCreate(); } \
    using Ptr = std::unique_ptr<T>;

	//----------------------------------------
	//        TypeListWrapper
	//----------------------------------------

	template<class T>
	class TypeListWrapper
	{
	public:
		TypeListValue& ValueList;

		TypeListWrapper(TypeListValue& valueList) : ValueList(valueList) {}

		// TODO, make this an internal iterator that returns the type wrapper so it can be de-referenced
		typename TypeValueList::iterator begin() { return ValueList.begin(); }
		typename TypeValueList::const_iterator begin() const { return ValueList.cbegin(); }

		typename TypeValueList::iterator end() { return ValueList.end(); }
		typename TypeValueList::const_iterator end() const { return ValueList.cend(); }

		T operator[] (size_t index)
		{
			return T(&ValueList[index]);
		}

		typename TypeValueList::iterator Erase(typename TypeValueList::iterator at) { return ValueList.Erase(at); }

		void Clear() { ValueList.Clear(); }

		bool IsEmpty() const { return ValueList.IsEmpty(); }

		size_t Size() const { return ValueList.Size(); }

		T PushBack()
		{
			return T(ValueList.PushBack(ValueList.GetType()));
		}
	};

	//----------------------------------------
	//        TypePtrListWrapper
	//----------------------------------------

	class TypePtrListWrapper
	{
	private:
		TypeListValue& ValueList;

	public:
		TypePtrListWrapper(TypeListValue& valueList) : ValueList(valueList) {}

		typename TypeValueList::iterator begin() { return ValueList.begin(); }
		typename TypeValueList::const_iterator begin() const { return ValueList.begin(); }

		typename TypeValueList::iterator end() { return ValueList.end(); }
		typename TypeValueList::const_iterator end() const { return ValueList.end(); }

		template<class T>
		T GetAs(size_t index)
		{
			return T(&ValueList[index]);
		}

		const TypeInfo* GetType(size_t index)
		{
			return ValueList[index].GetType();
		}

		typename TypeValueList::iterator Erase(typename TypeValueList::iterator at) { return ValueList.Erase(at); }

		void Clear() { ValueList.Clear(); }

		bool IsEmpty() const { return ValueList.IsEmpty(); }

		size_t Size() const { return ValueList.Size(); }

		template<class T>
		T PushBack()
		{
			return T(ValueList.PushBack(T::TypeName));
		}
	};
}