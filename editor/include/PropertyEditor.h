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

        PropertyEditor GetEditorForField(const Types::TypeInfo* type, int fieldIndex);

        void BuildCacheForType(const Types::TypeInfo* type, TypeEditorCache* cache);

    private:
        EditorSet BaseSet;
        std::vector<EditorSet> SetStack;

    private:
        PropertyEditor FindEditorByName(const std::string& name);
    };

    class TypeEditorCache
    {
    public:
        std::string TypeDisplayName;
        std::map<int, PropertyEditor> FieldEditors;
        std::map<int, TypeEditorCache> TypeEditors;

        void Clear()
        {
            TypeDisplayName.clear();
            FieldEditors.clear();
            TypeEditors.clear();
        }
    };

    // Primitive Editor Names
    static constexpr char EnumerationEditorName[] = "enum";
    static constexpr char IntEditorName[] = "int";
    static constexpr char FloatEditorName[] = "float";
    static constexpr char StringEditorName[] = "string";
}