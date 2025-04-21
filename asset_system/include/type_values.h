#pragma once

#include "type_database.h"
#include "type_events.h"
#include "type_field_value.h"
#include "primitive_type_value.h"
#include "field_path.h"
#include "Events.h"
#include <map>
#include <vector>
#include <iterator>

namespace Types
{
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
		TypeValue* GetParent() override;
		const TypeValue* GetParent() const override;
		void CallValueChanged(ValueChangedEvent& eventRecord);

		void SetType(const TypeInfo* type);

		const TypeInfo* GetType() const { return Type; }
		const TypeValueFieldMap& GetTypeFieldValues() const { return Values; }

		bool FieldIsDefault(int fieldIndex) const;

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
			eventRecord.RecordType = ValueChangedEvent::ValueRecordType::EnumerationChanged;
			eventRecord.Record = std::make_shared<EnumValueChangedRecord>();
			eventRecord.GetRecordAs<EnumValueChangedRecord>()->OldValue = valueItr->GetValue();
			valueItr->SetValueAs(value);
			eventRecord.GetRecordAs<EnumValueChangedRecord>()->NewValue = valueItr->GetValue();
			CallValueChanged(eventRecord);
		}

		inline void SetFieldEnumerationValueInt(int fieldIndex, const int32_t& value)
		{
			auto itr = Values.find(fieldIndex);
			if (itr == Values.end())
				itr = Values.insert_or_assign(fieldIndex, std::move(std::make_unique<EnumerationFieldValue>(this, SubPath + FieldPath::Field(fieldIndex)))).first;

			EnumerationFieldValue* valueItr = reinterpret_cast<EnumerationFieldValue*>(itr->second.get());

			ValueChangedEvent eventRecord;
			eventRecord.RecordType = ValueChangedEvent::ValueRecordType::EnumerationChanged;
			eventRecord.Record = std::make_shared<EnumValueChangedRecord>();
			eventRecord.GetRecordAs<EnumValueChangedRecord>()->OldValue = valueItr->GetValue();
			valueItr->SetValue(value);
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

		ListFieldValue& GetPrimitiveListFieldValue(int fieldIndex);
		int32_t GetListFieldCount(int fieldIndex);

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

			auto* list = (PrimitiveListFieldValue<T>*)itr->second;
			list->push_back(value);

			// add event
            ValueChangedEvent eventRecord;
			eventRecord.Path.PushFront(FieldPath::Index(list.Size() - 1));
			eventRecord.Path.PushFront(SubPath);
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::PrimitiveListItemAdded;
            eventRecord.Record = std::make_shared<PrimitiveListItemAddedRecord>();
            eventRecord.GetRecordAs<PrimitiveListItemAddedRecord>()->NewValue = value;
            CallValueChanged(eventRecord);
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

		void SetEnumFieldFromPath(const FieldPath& path, const int& value, int pathIndex = 0);

		std::string_view GetFieldNameFromPath(const FieldPath& path, int pathIndex = 0);
	};

	using TypeValueList = std::vector<TypeValue::Ptr>;

	class TypeListValue : public ListFieldValue
	{
	protected:
		const TypeInfo* Type = nullptr;
		TypeValueList Values;
	public:
		TypeListValue(TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : ListFieldValue(parentValue, path) {}
		TypeListValue(const TypeInfo* t, TypeValue* parentValue = nullptr, const FieldPath& path = FieldPath()) : ListFieldValue(parentValue, path) { Type = t; }

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

		typename std::vector<TypeValue::Ptr>::iterator Erase(typename std::vector<TypeValue::Ptr>::iterator at) 
		{ 
			return Values.erase(at);
		}

		void Clear() override 
		{ 
			Values.clear(); 

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath;
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::TypeListCleared;
            eventRecord.Record = std::make_shared<TypeListClearedRecord>();
            eventRecord.GetRecordAs<TypeListClearedRecord>()->OldValues = std::move(Values);

            CallValueChanged(eventRecord);
			Values.clear();
		}

		bool IsEmpty() const override { return Values.empty(); }

		size_t Size() const override { return Values.size(); }

		size_t Add() override
		{
			// push the known default type, make people insert pointer types
			PushBack(GetType());

			return Values.size() - 1;
		}

		void Delete(size_t index) override
		{
            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath + FieldPath::Index(int(index));
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::TypeListItemRemoved;
            eventRecord.Record = std::make_shared<TypeListItemRemovedRecord>();
            eventRecord.GetRecordAs<TypeListItemRemovedRecord>()->OldValue = std::move(Values[index]);
			Values.erase(Values.begin() + index);
			CallValueChanged(eventRecord);
		}

		TypeValue* PushBack(const TypeInfo* type)
		{
			FieldPath childPath = SubPath + FieldPath::Index(int(Values.size()) - 1);
			auto value = std::make_unique<TypeValue>(type, ParentValue, childPath);
			TypeValue* ret = value.get();
			Values.emplace_back(std::move(value));

            ValueChangedEvent eventRecord;
            eventRecord.Path = SubPath + FieldPath::Index(int(Values.size()-1));
            eventRecord.RecordType = ValueChangedEvent::ValueRecordType::TypeListItemAdded;
            eventRecord.Record = std::make_shared<TypeListItemAddedRecord>();
			eventRecord.GetRecordAs<TypeListItemAddedRecord>()->NewTypeID = type->TypeId;
			CallValueChanged(eventRecord);

			return ret;
		}

		TypeValue* PushBack(const std::string& typeName)
		{
			return PushBack(TypeDatabase::Get().FindType(typeName));
		}

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
	};
}
