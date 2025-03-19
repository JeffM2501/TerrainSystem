#pragma once

#include "Panel.h"

#include "TerrainBuilder.h"

class TerrainGenerationPanel : public EditorFramework::Panel
{
public:
    DEFINE_PANEL(TerrainGenerationPanel);

    TerrainGenerationPanel();

protected:
    void OnShow() override;

    TileMeshBuilder Builder;

private:
    int GridX = 6;
    int GridY = 6;

    float PerlinScale = 0.25f;
};
