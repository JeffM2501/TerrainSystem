#pragma once

#include "Document.h"

#include "TerrainTile.h"

#include "raylib.h"
#include <vector>

class TerrainDocument : public EditorFramework::Document
{
public:
    REGISTER_DOCUMENT(TerrainDocument, terrain);
    std::string_view GetDocumentName();
    void OnUpdate(int width, int height);
    void OnShowContent(int width, int height) override;
    void OnShowUI() override;
    void OnCreated() override;

    TerrainInfo Info;
    std::vector<TerrainTile> Tiles;

protected:
    Camera3D Camera = { 0 };
};