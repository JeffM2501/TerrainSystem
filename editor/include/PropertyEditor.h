#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "type_database.h"
#include "type_values.h"

namespace Properties
{
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

        PropertyEditor GetEditorForField(Types::TypeValue* value, int fieldIndex);

     //   TypeEditorCache BuildCacheForType(Types::TypeValue* value);

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
    };

    // Primitive Editor Names
    static constexpr char EnumerationEditorName[] = "enum";
    static constexpr char IntEditorName[] = "int";
    static constexpr char FloatEditorName[] = "float";
    static constexpr char StringEditorName[] = "string";
}