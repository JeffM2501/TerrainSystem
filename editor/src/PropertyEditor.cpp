#include "PropertyEditor.h"
#include "attributes.h"
#include "types/asset.h"

#include "imgui.h"
#include "imgui_utils.h"

#include "extras/IconsFontAwesome6.h"

#include "tinyfiledialogs.h"

using namespace Types;
using namespace AttributeTypes;
using namespace AssetTypes;
using namespace Hashes;

namespace Properties
{
	bool BoolPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		bool fieldValue = value->GetFieldPrimitiveValue<bool>(fieldIndex);

		const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		if (ImGui::Checkbox(label, &fieldValue))
		{
            value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
		}
		return false;
	}

	bool IntPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		const auto* primField = static_cast<const PrimitiveFieldInfo*>(value->GetType()->GetField(fieldIndex));

		int fieldValue = 0;
		switch (primField->GetPrimitiveType())
		{
		default:
			return false;
		case PrimitiveType::UInt8:
            fieldValue = value->GetFieldPrimitiveValue<uint8_t>(fieldIndex);
            break;
		case PrimitiveType::UInt16:
            fieldValue = value->GetFieldPrimitiveValue<uint16_t>(fieldIndex);
            break;
        case PrimitiveType::Int16:
            fieldValue = value->GetFieldPrimitiveValue<int16_t>(fieldIndex);
            break;
        case PrimitiveType::UInt32:
            fieldValue = value->GetFieldPrimitiveValue<uint32_t>(fieldIndex);
            break;
        case PrimitiveType::Int32:
            fieldValue = value->GetFieldPrimitiveValue<int32_t>(fieldIndex);
            break;
        case PrimitiveType::UInt64:
            fieldValue = int(value->GetFieldPrimitiveValue<uint64_t>(fieldIndex));
            break;
        case PrimitiveType::Int64:
            fieldValue = int(value->GetFieldPrimitiveValue<int64_t>(fieldIndex));
            break;
		}

        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
        if (ImGui::InputInt(label, &fieldValue))
        {
            switch (primField->GetPrimitiveType())
            {
            default:
                return false;
            case PrimitiveType::UInt8:
				value->SetFieldPrimitiveValue<uint8_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::UInt16:
				value->SetFieldPrimitiveValue<uint16_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::Int16:
				value->SetFieldPrimitiveValue<int16_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::UInt32:
				value->SetFieldPrimitiveValue<uint32_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::Int32:
				value->SetFieldPrimitiveValue<int32_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::UInt64:
				value->SetFieldPrimitiveValue<uint64_t>(fieldIndex, fieldValue);
                break;
            case PrimitiveType::Int64:
				value->SetFieldPrimitiveValue<int64_t>(fieldIndex, fieldValue);
                break;
            }
        }
        return false;
	}

	bool FloatPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        const auto* primField = static_cast<const PrimitiveFieldInfo*>(value->GetType()->GetField(fieldIndex));

		float fieldValue = 0.0f;
		switch (primField->GetPrimitiveType())
		{
		default:
			return false;
		case PrimitiveType::Float32:
			fieldValue = value->GetFieldPrimitiveValue<float>(fieldIndex);
			break;

		case PrimitiveType::Double64:
			fieldValue = float(value->GetFieldPrimitiveValue<double>(fieldIndex));
			break;
		}

        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
        if (ImGui::InputFloat(label, &fieldValue))
        {
            switch (primField->GetPrimitiveType())
            {
            default:
                return false;
            case PrimitiveType::Float32:
				value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
                break;

            case PrimitiveType::Double64:
				value->SetFieldPrimitiveValue<double>(fieldIndex, fieldValue);
                break;
            }
        }
        return false;
	}

	bool StringPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        std::string fieldValue = value->GetFieldPrimitiveValue<std::string>(fieldIndex);

        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		char buffer[256] = { 0 };
		strcpy(buffer, fieldValue.c_str());
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText(label, buffer, 256))
        {
			fieldValue = buffer;

            value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
        return false;
	}

	bool EnumPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
		if (!value->GetType()->GetField(fieldIndex)->IsEnum())
			return false;

		const EnumerationFieldInfo* enumInfo = static_cast<const EnumerationFieldInfo*>(value->GetType()->GetField(fieldIndex));
		auto * enumTypeInfo = TypeDatabase::Get().FindEnumeration(enumInfo->TypePtr->TypeName);

        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		int valueIndex = value->GetFieldEnumerationValue<int>(fieldIndex);

		if (ImGui::BeginCombo(label, enumTypeInfo->Values[valueIndex].c_str(), ImGuiComboFlags_None))
		{
			for (auto& [index, name] : enumTypeInfo->Values)
			{
				bool selected = index == valueIndex;

				if (ImGui::Selectable(name.c_str(), &selected))
				{
					valueIndex = index;
				}
			}
			ImGui::EndCombo();
		}

		if (valueIndex != value->GetFieldEnumerationValue<int>(fieldIndex))
		{
			value->SetFieldEnumerationValue<int>(fieldIndex, valueIndex);
		}

		return false;
	}

	bool VecFloatEditor(const char* label, float& value, ImVec4 color)
	{
        ImGui::TableNextColumn();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool dirty = ImGui::DragFloat(TextFormat("###%s", label), &value);
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
            ImVec2(ImGui::GetItemRectMin().x + 3,
                ImGui::GetItemRectMax().y),
            ImGui::ColorConvertFloat4ToU32(color));

		return dirty;
	}

	bool Vec2PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        Vector2 fieldValue = value->GetFieldPrimitiveValue<Vector2>(fieldIndex);

		if (ImGui::BeginTable("Vec2Editor", 2))
		{
			ImGui::TableNextRow();

			bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1));
			dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1)) || dirty;
			ImGui::EndTable();

			if (dirty)
				value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
		}
		return false;
	}

	bool Vec3PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        Vector3 fieldValue = value->GetFieldPrimitiveValue<Vector3>(fieldIndex);

        if (ImGui::BeginTable("Vec3Editor", 3))
        {
            ImGui::TableNextRow();

            bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1));
            dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1)) || dirty;
			dirty = VecFloatEditor("Z", fieldValue.z, ImVec4(0, 0, 1, 1)) || dirty;
            ImGui::EndTable();

            if (dirty)
                value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
        return false;
	}

	bool Vec4PropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        Vector4 fieldValue = value->GetFieldPrimitiveValue<Vector4>(fieldIndex);

        if (ImGui::BeginTable("Vec4Editor", 4))
        {
            ImGui::TableNextRow();

            bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1));
            dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1)) || dirty;
            dirty = VecFloatEditor("Z", fieldValue.z, ImVec4(0, 0, 1, 1)) || dirty;
			dirty = VecFloatEditor("W", fieldValue.w, ImVec4(1, 0, 1, 1)) || dirty;
            ImGui::EndTable();

            if (dirty)
                value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
        return false;
	}

	bool RectPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        Rectangle fieldValue = value->GetFieldPrimitiveValue<Rectangle>(fieldIndex);

        if (ImGui::BeginTable("RectEditor", 4))
        {
            ImGui::TableNextRow();

            bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1));
            dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1)) || dirty;
            dirty = VecFloatEditor("W", fieldValue.width, ImVec4(1, 0.5f, 0.5f, 1)) || dirty;
            dirty = VecFloatEditor("H", fieldValue.height, ImVec4(0.5f, 1, 0.5f, 1)) || dirty;
            ImGui::EndTable();

            if (dirty)
                value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
        return false;
	}

	bool ColorPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        Color fieldValue = value->GetFieldPrimitiveValue<Color>(fieldIndex);

        const char* label = TextFormat("###%s_button", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		float color[4] = { fieldValue.r / 255.0f, fieldValue.g / 255.0f, fieldValue.b / 255.0f , fieldValue.a / 255.0f };

		ImVec4 color4 = { color[0], color[1], color[2], 1 };
		ImGui::ColorButton(label, color4);
		ImGui::SameLine();

		label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        if (ImGui::ColorEdit4(label, color, ImGuiColorEditFlags_NoSmallPreview))
        {
			fieldValue.r = uint8_t(color[0] * 255);
			fieldValue.g = uint8_t(color[1] * 255);
			fieldValue.b = uint8_t(color[2] * 255);
			fieldValue.a = uint8_t(color[3] * 255);

            value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
		return false;
	}

	bool MatrixPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        Matrix fieldValue = value->GetFieldPrimitiveValue<Matrix>(fieldIndex);
        const char* label = TextFormat("###%s_r1", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		ImGui::Dummy(ImVec2(10, 1)); ImGui::SameLine();
        ImGui::InputFloat4(label, &fieldValue.m0);

        label = TextFormat("###%s_r2", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		ImGui::Dummy(ImVec2(10, 1)); ImGui::SameLine(); 
        ImGui::InputFloat4(label, &fieldValue.m1);

        label = TextFormat("###%s_r3", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		ImGui::Dummy(ImVec2(10, 1)); ImGui::SameLine(); 
        ImGui::InputFloat4(label, &fieldValue.m2);

        label = TextFormat("###%s_r4", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		ImGui::Dummy(ImVec2(10, 1)); ImGui::SameLine(); 
        ImGui::InputFloat4(label, &fieldValue.m3);
		return false;
	}

	bool GuidPropertyEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
	{
        GUID fieldValue = value->GetFieldPrimitiveValue<GUID>(fieldIndex);

        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

        char buffer[256] = { 0 };
        strcpy(buffer, fieldValue.ToString().c_str());
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText(label, buffer, 256))
        {
			fieldValue = GUID::FromString(buffer);

            value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
        }
        return false;
	}

	const char* AssetRefPickerDialog(Types::TypeValue* value, int fieldIndex)
	{
        const char* filters[1] = { nullptr };

        if (auto* extAttr = value->GetType()->GetFieldAttribute<AssetTypes::FileExtensionAttribute>(fieldIndex))
        {
            filters[0] = TextFormat("*.%s", extAttr->Extension.c_str());
        }
        else
        {
            filters[0] = "*.*";
        }
        return tinyfd_openFileDialog("Select file...", nullptr, 1, filters, nullptr, false);
	}

    bool ResourceReferenceEditor(Types::TypeValue* value, int fieldIndex, bool expanded)
    {
        AssetTypes::ResourceReference ref(value->GetTypeFieldValue(fieldIndex));
        const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
        char buffer[256] = { 0 };
        strcpy(buffer, ref.GetPath().c_str());

		auto buttonSize = ImGuiUtils::GetButtonsSize(ICON_FA_ELLIPSIS, ICON_FA_FOLDER_OPEN, "");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonSize.x - ImGui::GetStyle().ItemInnerSpacing.x);

		if (ImGui::InputText(label, buffer, 256))
			ref.SetPath(buffer);

        ImGui::SameLine();
		if (ImGui::Button(ICON_FA_ELLIPSIS))
		{
            auto fileToOpen = AssetRefPickerDialog(value, fieldIndex);
            if (fileToOpen)
				ref.SetPath(fileToOpen);
        }   

		ImGui::SameLine();
		ImGui::BeginDisabled(ref.GetPath().empty());
        if (ImGui::Button(ICON_FA_FOLDER_OPEN))
        {
			OpenURL(TextFormat("file://%s", buffer));
        }
        ImGui::EndDisabled();
			
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

		BaseSet[ColorEditorName] = ColorPropertyEditor;

        BaseSet["ResourceReferenceEditor"] = ResourceReferenceEditor;
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

		case Types::PrimitiveType::Color:
            return ColorEditorName;

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

	std::string GetDisplayName(const std::string& name)
	{
		std::string newName;

		for (size_t i = 0; i < name.size(); i++)
		{
            if (i != 0 && std::isupper(name[i]) && !std::isupper(name[i - 1]))
                newName += " ";

			newName += name[i];
		}

		return newName;
	}

	void EditorRegistry::BuildCacheForType(Types::TypeValue* value, TypeEditorCache* cache) const
	{
		if (value == nullptr)
			return;

		auto* type = value->GetType();

		if (!type)
			return;

		cache->TypeDisplayName = type->TypeName;
		if (type->HasAttribute<DisplayNameAttribute>())
			cache->TypeDisplayName = type->GetAttribute<DisplayNameAttribute>()->Name;

		cache->TypeDisplayName = GetDisplayName(cache->TypeDisplayName);

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

				fieldCache.DisplayName = GetDisplayName(fieldCache.DisplayName);

				cache->FieldEditors[i] = fieldCache;
			}
			else if (fieldInfo->IsType())
			{
                auto& typeCache = cache->TypeEditors.try_emplace(i).first->second;
                typeCache.DisplayName = fieldInfo->GetName();
                typeCache.DisplayName = GetDisplayName(typeCache.DisplayName);

				if (type->FieldHasAttribute<AttributeTypes::CustomEditorAttribute>(i))
				{
                    auto customEditor = type->GetFieldAttribute<AttributeTypes::CustomEditorAttribute>(i)->EditorName;
					auto editor = FindEditorByName(customEditor);
                    if (editor)
                    {
                        typeCache.CustomEditor = editor;
                        continue;
                    }
				}

				if (fieldInfo->GetType() == Types::FieldType::TypeList)
				{
					int index = 0;
					for (auto& listValue : value->GetTypeListFieldValue(i))
					{
						TypeEditorCache cacheRecord;
						BuildCacheForType(listValue.get(), &cacheRecord);
						typeCache.Editors.push_back(cacheRecord);

						cacheRecord.TypeDisplayName = TextFormat("%s %d",typeCache.DisplayName.c_str(), i);
					}
				}
				else
				{
					TypeEditorCache cacheRecord;
					auto* fieldValue = value->GetTypeFieldValue(i);
					BuildCacheForType(fieldValue, &cacheRecord);
					cacheRecord.TypeDisplayName = typeCache.DisplayName;

					typeCache.Editors.push_back(cacheRecord);
				}
			}
		}
	}
}