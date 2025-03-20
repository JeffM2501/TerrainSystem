#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "type_database.h"
#include "type_values.h"

namespace Properties
{
	class TypeEditorCache;

	using PropertyEditor = std::function<bool(Types::TypeValue* value, int fieldIndex, bool expanded)>;

	class EditorSet : public std::unordered_map<std::string, PropertyEditor>
	{
	public:
	};

	class EditorRegistry
	{
	public:
		EditorRegistry();

		EditorSet& PushSet();
		void PopSet();

		PropertyEditor GetEditorForType(const Types::TypeInfo* type, int fieldIndex) const;

		void BuildCacheForType(Types::TypeValue* type, TypeEditorCache* cache) const;

	private:
		EditorSet BaseSet;
		std::vector<EditorSet> SetStack;

	private:
		PropertyEditor FindEditorByName(const std::string& name) const;
	};

	class TypeEditorCache
	{
	public:
		std::string TypeDisplayName;

		class PrimitiveFieldCacheInfo
		{
		public:
			std::string DisplayName;
			PropertyEditor Editor;
		};

		std::map<int, PrimitiveFieldCacheInfo> FieldEditors;

		std::map<int, std::vector<TypeEditorCache>> TypeEditors;

		void Clear()
		{
			TypeDisplayName.clear();
			FieldEditors.clear();
			TypeEditors.clear();
		}
	};

	// Primitive Editor Names
	static constexpr char EnumerationEditorName[] = "enum";
	static constexpr char BoolEditorName[] = "bool";
	static constexpr char IntEditorName[] = "int";
	static constexpr char FloatEditorName[] = "float";
	static constexpr char StringEditorName[] = "string";

	static constexpr char Vec2EditorName[] = "vec2";
	static constexpr char Vec3EditorName[] = "vec3";
	static constexpr char Vec4EditorName[] = "vec4";
	static constexpr char RectEditorName[] = "rect";
	static constexpr char ColorEditorName[] = "color";
	static constexpr char MatrixEditorName[] = "matrix";
	static constexpr char GUIDEditorName[] = "GUID";
}