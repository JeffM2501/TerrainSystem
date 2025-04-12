#pragma once

#include "Panel.h"

#include "type_values.h"
#include "PropertyEditor.h"
#include "AssetEditManager.h"

class PropertiesPanel : public EditorFramework::Panel
{
public:
    DEFINE_PANEL(PropertiesPanel);

    PropertiesPanel();

    void SetObject(Types::TypeValue* value);

protected:
    void OnShow() override;

    Types::TypeValue* ObjectPointer = nullptr;

    Properties::TypeEditorCache TypeCache;

    Properties::EditorRegistry Registry;
};
