#pragma once

#include "Panel.h"


class TerrainInfoPanel : public EditorFramework::Panel
{
public:
    TerrainInfoPanel();

protected:
    void OnShow() override;
};
