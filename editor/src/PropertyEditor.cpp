#include "PropertyEditor.h"
#include "attributes.h"
#include "types/asset.h"
#include "AssetManager.h"

#include "imgui.h"
#include "imgui_utils.h"
#include "rlImGui.h"

#include "extras/IconsFontAwesome6.h"

#include "tinyfiledialogs.h"

#include "EditorThumbnailManager.h"

using namespace Types;
using namespace AttributeTypes;
using namespace AssetTypes;
using namespace AssetSystem;
using namespace Hashes;
using namespace Editor;

namespace Properties
{
	bool BoolPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		bool fieldValue = false;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();

		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<bool>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<bool>(fieldIndex);

		const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		if (ImGui::Checkbox(label, &fieldValue))
		{
			if (isList)
				value->GetPrimitiveListFieldValue<bool>(fieldIndex).SetValue(fieldValue, arrayIndex);
			else
				value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
		}
		return false;
	}

	bool IntPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		const auto* primField = static_cast<const PrimitiveFieldInfo*>(value->GetType()->GetField(fieldIndex));
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		int fieldValue = 0;
		switch (primField->GetPrimitiveType())
		{
		default:
			return false;
		case PrimitiveType::UInt8:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<uint8_t>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<uint8_t>(fieldIndex);
			break;
		case PrimitiveType::UInt16:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<uint16_t>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<uint16_t>(fieldIndex);
			break;
		case PrimitiveType::Int16:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<int16_t>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<int16_t>(fieldIndex);
			break;
		case PrimitiveType::UInt32:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<uint32_t>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<uint32_t>(fieldIndex);
			break;
		case PrimitiveType::Int32:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<int32_t>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<int32_t>(fieldIndex);
			break;
		case PrimitiveType::UInt64:
			if (isList)
				fieldValue = int(value->GetPrimitiveListFieldValue<uint64_t>(fieldIndex).GetValue(arrayIndex));
			else
				fieldValue = int(value->GetFieldPrimitiveValue<uint64_t>(fieldIndex));
			break;
		case PrimitiveType::Int64:
			if (isList)
				fieldValue = int(value->GetPrimitiveListFieldValue<int64_t>(fieldIndex).GetValue(arrayIndex));
			else
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
				if (isList)
					value->GetPrimitiveListFieldValue<uint8_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<uint8_t>(fieldIndex, fieldValue);

				break;
			case PrimitiveType::UInt16:
				if (isList)
					value->GetPrimitiveListFieldValue<uint16_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<uint16_t>(fieldIndex, fieldValue);
				break;
			case PrimitiveType::Int16:
				if (isList)
					value->GetPrimitiveListFieldValue<int16_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<int16_t>(fieldIndex, fieldValue);
				break;
			case PrimitiveType::UInt32:
				if (isList)
					value->GetPrimitiveListFieldValue<uint32_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<uint32_t>(fieldIndex, fieldValue);
				break;
			case PrimitiveType::Int32:
				if (isList)
					value->GetPrimitiveListFieldValue<int32_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<int32_t>(fieldIndex, fieldValue);
				break;
			case PrimitiveType::UInt64:
				if (isList)
					value->GetPrimitiveListFieldValue<uint64_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<uint64_t>(fieldIndex, fieldValue);
				break;
			case PrimitiveType::Int64:
				if (isList)
					value->GetPrimitiveListFieldValue<int64_t>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<int64_t>(fieldIndex, fieldValue);
				break;
			}
		}
		return false;
	}

	bool FloatPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		const auto* primField = static_cast<const PrimitiveFieldInfo*>(value->GetType()->GetField(fieldIndex));

		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		float fieldValue = 0.0f;
		switch (primField->GetPrimitiveType())
		{
		default:
			return false;
		case PrimitiveType::Float32:
			if (isList)
				fieldValue = value->GetPrimitiveListFieldValue<float>(fieldIndex).GetValue(arrayIndex);
			else
				fieldValue = value->GetFieldPrimitiveValue<float>(fieldIndex);
			break;

		case PrimitiveType::Double64:
			if (isList)
				fieldValue = float(value->GetPrimitiveListFieldValue<double>(fieldIndex).GetValue(arrayIndex));
			else
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
				if (isList)
					value->GetPrimitiveListFieldValue<float>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue(fieldIndex, fieldValue);
				break;

			case PrimitiveType::Double64:
				if (isList)
					value->GetPrimitiveListFieldValue<double>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<double>(fieldIndex, fieldValue);
				break;
			}
		}
		return false;
	}

	bool StringPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		std::string fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<std::string>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<std::string>(fieldIndex);

		const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		char buffer[256] = { 0 };
		strcpy(buffer, fieldValue.c_str());
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputText(label, buffer, 256))
		{
			fieldValue = buffer;

			if (isList)
				value->GetPrimitiveListFieldValue<std::string>(fieldIndex).SetValue(fieldValue, arrayIndex);
			else
				value->SetFieldPrimitiveValue<std::string>(fieldIndex, fieldValue);
		}
		return false;
	}

	bool EnumPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		if (!value->GetType()->GetField(fieldIndex)->IsEnum())
			return false;

		const EnumerationFieldInfo* enumInfo = static_cast<const EnumerationFieldInfo*>(value->GetType()->GetField(fieldIndex));
		auto* enumTypeInfo = TypeDatabase::Get().FindEnumeration(enumInfo->TypePtr->TypeName);

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

	bool VecFloatEditor(const char* label, float& value, ImVec4 color, bool showLabel)
	{
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		if (showLabel)
		{
			ImGui::TextUnformatted(label);
			ImGui::SameLine();
		}
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		bool dirty = ImGui::DragFloat(TextFormat("###%s", label), &value);

		if (!showLabel)
			ImGui::SetItemTooltip("%s", label);

		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
			ImVec2(ImGui::GetItemRectMin().x + 3,
				ImGui::GetItemRectMax().y),
			ImGui::ColorConvertFloat4ToU32(color));

		return dirty;
	}

	bool Vec2PropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Vector2 fieldValue;

		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Vector2>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Vector2>(fieldIndex);

		ImGui::PushStyleVarX(ImGuiStyleVar_CellPadding, 0);
		if (ImGui::BeginTable("Vec2Editor", 2))
		{
			ImGui::TableNextRow();

			bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1), false);
			dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1), false) || dirty;
			ImGui::EndTable();

			if (dirty)
			{
				if (isList)
					value->GetPrimitiveListFieldValue<Vector2>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<Vector2>(fieldIndex, fieldValue);
			}
		}
		ImGui::PopStyleVar();
		return false;
	}

	bool Vec3PropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Vector3 fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Vector3>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Vector3>(fieldIndex);

		ImGui::PushStyleVarX(ImGuiStyleVar_CellPadding, 0);
		if (ImGui::BeginTable("Vec3Editor", 3))
		{
			ImGui::TableNextRow();

			bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1), false);
			dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1), false) || dirty;
			dirty = VecFloatEditor("Z", fieldValue.z, ImVec4(0, 0, 1, 1), false) || dirty;
			ImGui::EndTable();

			if (dirty)
			{
				if (isList)
					value->GetPrimitiveListFieldValue<Vector3>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<Vector3>(fieldIndex, fieldValue);
			}
		}
		ImGui::PopStyleVar();
		return false;
	}

	bool Vec4PropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Vector4 fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Vector4>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Vector4>(fieldIndex);

		ImGui::PushStyleVarX(ImGuiStyleVar_CellPadding, 0);
		if (ImGui::BeginTable("Vec4Editor", 4))
		{
			ImGui::TableNextRow();

			bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1), false);
			dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1), false) || dirty;
			dirty = VecFloatEditor("Z", fieldValue.z, ImVec4(0, 0, 1, 1), false) || dirty;
			dirty = VecFloatEditor("W", fieldValue.w, ImVec4(1, 0, 1, 1), false) || dirty;
			ImGui::EndTable();

			if (dirty)
			{
				if (isList)
					value->GetPrimitiveListFieldValue<Vector4>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<Vector4>(fieldIndex, fieldValue);
			}
		}
		ImGui::PopStyleVar();
		return false;
	}

	bool RectPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Rectangle fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Rectangle>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Rectangle>(fieldIndex);

		ImGui::PushStyleVarX(ImGuiStyleVar_CellPadding, 0);
		if (ImGui::BeginTable("RectEditor", 4))
		{
			ImGui::TableNextRow();

			bool dirty = VecFloatEditor("X", fieldValue.x, ImVec4(1, 0, 0, 1), false);
			dirty = VecFloatEditor("Y", fieldValue.y, ImVec4(0, 1, 0, 1), false) || dirty;
			dirty = VecFloatEditor("Width", fieldValue.width, ImVec4(1, 0.5f, 0.5f, 1), false) || dirty;
			dirty = VecFloatEditor("Height", fieldValue.height, ImVec4(0.5f, 1, 0.5f, 1), false) || dirty;
			ImGui::EndTable();

			if (dirty)
			{
				if (isList)
					value->GetPrimitiveListFieldValue<Rectangle>(fieldIndex).SetValue(fieldValue, arrayIndex);
				else
					value->SetFieldPrimitiveValue<Rectangle>(fieldIndex, fieldValue);
			}
		}
		ImGui::PopStyleVar();
		return false;
	}

	bool ColorPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Color fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Color>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Color>(fieldIndex);

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

			if (isList)
				value->GetPrimitiveListFieldValue<Color>(fieldIndex).SetValue(fieldValue, arrayIndex);
			else
				value->SetFieldPrimitiveValue<Color>(fieldIndex, fieldValue);
		}
		return false;
	}

	bool MatrixPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		Matrix fieldValue;

		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<Matrix>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<Matrix>(fieldIndex);

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

	bool GuidPropertyEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		GUID fieldValue;
		bool isList = value->GetType()->GetField(fieldIndex)->IsList();
		if (isList && arrayIndex >= value->GetListFieldCount(fieldIndex))
			return false;

		if (isList)
			fieldValue = value->GetPrimitiveListFieldValue<GUID>(fieldIndex).GetValue(arrayIndex);
		else
			fieldValue = value->GetFieldPrimitiveValue<GUID>(fieldIndex);

		const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());

		char buffer[256] = { 0 };
		strcpy(buffer, fieldValue.ToString().c_str());
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::InputText(label, buffer, 256))
		{
			fieldValue = GUID::FromString(buffer);

			if (isList)
				value->GetPrimitiveListFieldValue<GUID>(fieldIndex).SetValue(fieldValue, arrayIndex);
			else
				value->SetFieldPrimitiveValue<GUID>(fieldIndex, fieldValue);
		}
		return false;
	}

	AssetManager::AssetPath AssetRefPickerDialog(Types::TypeValue* value, int fieldIndex, std::string_view filePath)
	{
		const char* filters[1] = { nullptr };

		std::string initalPath(filePath);

		if (auto* extAttr = value->GetType()->GetFieldAttribute<AssetTypes::FileExtensionAttribute>(fieldIndex))
		{
			filters[0] = TextFormat("*.%s", extAttr->Extension.c_str());

			if (initalPath.empty())
				initalPath = filters[0];
		}
		else
		{
			filters[0] = "*.*";
		}

		const char* file = tinyfd_openFileDialog("Select file...", initalPath.c_str(), 1, filters, "Asset File", false);

		AssetManager::AssetPath path;
		if (file)
			path = AssetManager::ToAssetPath(AssetManager::FileSystemPath(file));

		return path;
	}

	bool ResourceReferenceEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		AssetTypes::ResourceReference ref(value->GetTypeFieldValue(fieldIndex));
		const char* label = TextFormat("###%s", value->GetType()->GetField(fieldIndex)->GetName().c_str());
		char buffer[256] = { 0 };
		strcpy(buffer, ref.GetPath().c_str());

		auto buttonSize = ImGuiUtils::GetButtonsSize(ICON_FA_ELLIPSIS, ICON_FA_FOLDER_OPEN, ICON_FA_TRASH);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonSize.x - ImGui::GetStyle().ItemInnerSpacing.x);

		if (ImGui::InputText(label, buffer, 256))
			ref.SetPath(buffer);

		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_ELLIPSIS))
		{
			auto fileToOpen = AssetRefPickerDialog(value, fieldIndex, buffer);
			if (!fileToOpen.empty())
				ref.SetPath(fileToOpen);
		}
		ImGui::SetItemTooltip("Select Resource File");

		ImGui::SameLine();
		ImGui::BeginDisabled(ref.GetPath().empty());
		if (ImGui::Button(ICON_FA_FILE_PEN))
		{
			AssetManager::FileSystemPath filePath = AssetManager::ToFileSystemPath(AssetManager::AssetPath(buffer));

			OpenURL(TextFormat("file://%s", filePath.c_str()));
		}
		ImGui::SetItemTooltip("Show in explorer");
		ImGui::EndDisabled();


        ImGui::SameLine();
        ImGui::BeginDisabled(ref.GetPath().empty());
        if (ImGui::Button(ICON_FA_TRASH))
        {
			ref.ResetPath();
        }
        ImGui::SetItemTooltip("Clear resource reference");
		ImGui::EndDisabled();

		return false;
	}

	bool TextureReferenceEditor(Types::TypeValue* value, int fieldIndex, int arrayIndex)
	{
		AssetTypes::ResourceReference ref(value->GetTypeFieldValue(fieldIndex));

		ImGui::PushID(value);
		Vector2 size{ ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };
		auto& texturePath = ref.GetPath();
		if (!texturePath.empty())
		{
			auto texture = ThumbnailManager::GetThumbnail(texturePath);
			rlImGuiImageSize(&texture, int(size.x), int(size.y));

			if (ImGui::IsItemHovered())
			{
				if (ImGui::BeginTooltip())
				{
					int x = std::min(texture.width, 256);
					int y = std::min(texture.height, 256);

					rlImGuiImageSize(&texture, x, y);
					ImGui::EndTooltip();
				}
			}
			ImGui::SameLine(0, 0);
		}
		ImGui::PopID();

		return ResourceReferenceEditor(value, fieldIndex, arrayIndex);
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

		BaseSet[std::string(AssetTypes::ResourceReferenceEditor)] = ResourceReferenceEditor;
		BaseSet[std::string(AssetTypes::TextureReferenceEditor)] = TextureReferenceEditor;
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

	void EditorRegistry::BuildCacheForListField(Types::TypeValue* value, TypeEditorCache* cache, int fieldIndex) const
	{
		cache->TypeEditors[fieldIndex].Editors.clear();

		auto& typeList = value->GetTypeListFieldValue(fieldIndex);

		for (int i = 0; i < typeList.Size(); i++)
		{
			cache->TypeEditors[fieldIndex].Editors.push_back(TypeEditorCache());
			BuildCacheForType(&typeList[i], &cache->TypeEditors[fieldIndex].Editors[i]);
        }
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

				fieldCache.IsList = fieldInfo->IsList();

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

						cacheRecord.TypeDisplayName = TextFormat("%s %d", typeCache.DisplayName.c_str(), i);
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