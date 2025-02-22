#pragma once

#include "Panel.h"

class TerrainGenerationPanel : public EditorFramework::Panel
{
public:
    TerrainGenerationPanel();

protected:
    void OnShow() override;

private:
    int GridX = 12;
    int GridY = 12;

    float PerlinScale = 2;
};
