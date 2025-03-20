#include "PropertyEditor.h"
#include "attributes.h"

using namespace Types;
using namespace AttributeTypes;

namespace Properties
{

	bool BoolPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool IntPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool FloatPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool StringPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool EnumPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool Vec2PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool Vec3PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool Vec4PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool RectPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool ColorPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool MatrixPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}

	bool GuidPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		return false;
	}


	EditorRegistry::EditorRegistry()
	{
		BaseSet[BoolEditorName] = BoolPropertyEditor;
		BaseSet[IntEditorName] = IntPropertyEditor;
		BaseSet[FloatEditorName] = FloatPropertyEditor;
		BaseSet[StringEditorName] = StringPropertyEditor;
		BaseSet[EnumerationEditorName] = EnumPropertyEditor;

		BaseSet[Vec2EditorName] = Vec2PropertyEditor;
		BaseSet[Vec3EditorName] = Vec3PropertyEditor;
		BaseSet[Vec4EditorName] = Vec4PropertyEditor;
		BaseSet[RectEditorName] = RectPropertyEditor;
		BaseSet[ColorEditorName] = ColorPropertyEditor;
		BaseSet[MatrixEditorName] = MatrixPropertyEditor;
		BaseSet[GUIDEditorName] = GuidPropertyEditor;
	}

	const char* GetPrimitveEditorName(PrimitiveType primType)
	{
		switch (primType)
		{
		case Types::PrimitiveType::Bool:
			return BoolEditorName;

		case Types::PrimitiveType::Char:
		case Types::PrimitiveType::UInt8:
		case Types::PrimitiveType::UInt16:
		case Types::PrimitiveType::Int16:
		case Types::PrimitiveType::UInt32:
		case Types::PrimitiveType::Int32:
		case Types::PrimitiveType::UInt64:
		case Types::PrimitiveType::Int64:
			return IntEditorName;

		case Types::PrimitiveType::Float32:
		case Types::PrimitiveType::Double64:
			return FloatEditorName;

		case Types::PrimitiveType::String:
			return StringEditorName;

		case Types::PrimitiveType::Vector2:
			return Vec2EditorName;

		case Types::PrimitiveType::Vector3:
			return Vec3EditorName;

		case Types::PrimitiveType::Vector4:
			return Vec4EditorName;

		case Types::PrimitiveType::Rectangle:
			return RectEditorName;

		case Types::PrimitiveType::Matrix:
			return MatrixEditorName;

		case Types::PrimitiveType::GUID:
			return GUIDEditorName;

		case Types::PrimitiveType::Unknown:
		default:
			return nullptr;
		}

		return nullptr;
	}

	const char* GetNameForField(const Types::FieldInfo* fieldInfo)
	{
		switch (fieldInfo->GetType())
		{
		default:
			return nullptr;

		case FieldType::Primitive:
		case FieldType::PrimitiveList:
		{
			const PrimitiveFieldInfo* primitiveField = static_cast<const PrimitiveFieldInfo*>(fieldInfo);

			return GetPrimitveEditorName(primitiveField->GetPrimitiveType());
		}

		case FieldType::Type:
		case FieldType::TypeList:
		{
			const TypeFieldInfo* typeField = static_cast<const TypeFieldInfo*>(fieldInfo);

			return typeField->TypePtr->TypeName.c_str();
		}

		case FieldType::Enumeration:
			return EnumerationEditorName;
		}
	}


	EditorSet& EditorRegistry::PushSet()
	{
		SetStack.emplace_back();
		return SetStack.back();
	}

	void EditorRegistry::PopSet()
	{
		SetStack.pop_back();
	}

	PropertyEditor EditorRegistry::FindEditorByName(const std::string& name) const
	{
		for (auto itr = SetStack.rbegin(); itr != SetStack.rend(); itr--)
		{
			auto editItr = itr->find(name);
			if (editItr != itr->end())
				return editItr->second;
		}

		auto editItr = BaseSet.find(name);
		if (editItr != BaseSet.end())
			return editItr->second;

		return nullptr;
	}

	PropertyEditor EditorRegistry::GetEditorForType(const Types::TypeInfo* type, int fieldIndex) const
	{
		const Types::FieldInfo* fieldInfo = type->GetField(fieldIndex);

		const auto* customEditor = type->GetFieldAttribute<CustomEditorAttribute>(fieldIndex);

		if (customEditor)
		{
			auto editor = FindEditorByName(customEditor->EditorName);
			if (editor)
				return editor;
		}

		const char* fieldEditorName = GetNameForField(fieldInfo);
		if (fieldEditorName == nullptr)
			return nullptr;

		return FindEditorByName(fieldEditorName);
	}

	void EditorRegistry::BuildCacheForType(Types::TypeValue* value, TypeEditorCache* cache) const
	{
		auto* type = value->GetType();

		if (!type)
			return;

		cache->TypeDisplayName = type->TypeName;
		if (type->HasAttribute<DisplayNameAttribute>())
			cache->TypeDisplayName = type->GetAttribute<DisplayNameAttribute>()->Name;

		for (int i = 0; i < type->GetFieldCount(); i++)
		{
			auto fieldInfo = type->GetField(i);

			if (type->FieldHasAttribute<HiddenAttribute>(i))
				continue;

			if (fieldInfo->IsPrimtive() || fieldInfo->IsEnum())
			{
				TypeEditorCache::PrimitiveFieldCacheInfo fieldCache;
				fieldCache.Editor = GetEditorForType(type, i);

				if (!fieldCache.Editor)
					continue;

				fieldCache.DisplayName = fieldInfo->GetName();
				if (type->FieldHasAttribute<DisplayNameAttribute>(i))
					fieldCache.DisplayName = type->GetFieldAttribute<DisplayNameAttribute>(i)->Name;

				cache->FieldEditors[i] = fieldCache;
			}
			else if (fieldInfo->IsType())
			{
				auto& typeCache = cache->TypeEditors.try_emplace(i).first->second;

				if (fieldInfo->GetType() == Types::FieldType::TypeList)
				{
					for (auto& listValue : value->GetTypeListFieldValue(i))
					{
						TypeEditorCache cacheRecord;
						BuildCacheForType(listValue.get(), &cacheRecord);
						typeCache.push_back(cacheRecord);
					}
				}
				else
				{
					TypeEditorCache cacheRecord;
					auto* fieldValue = value->GetTypeFieldValue(i);
					BuildCacheForType(fieldValue, &cacheRecord);
					typeCache.push_back(cacheRecord);
				}
			}
		}
	
	}
}