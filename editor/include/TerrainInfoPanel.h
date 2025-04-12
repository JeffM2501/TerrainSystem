#pragma once

#include "Panel.h"

class TerrainInfoPanel : public EditorFramework::Panel
{
public:
	DEFINE_PANEL(TerrainInfoPanel);

    TerrainInfoPanel();

protected:
    void OnShow() override;
};
