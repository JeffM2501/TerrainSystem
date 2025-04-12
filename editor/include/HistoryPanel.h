#pragma once

#include "Panel.h"

#include "type_values.h"
#include "PropertyEditor.h"
#include "AssetEditManager.h"

class HistoryPanel : public EditorFramework::Panel
{
public:
    DEFINE_PANEL(HistoryPanel);

    HistoryPanel();

protected:
    void OnShow() override;
};
