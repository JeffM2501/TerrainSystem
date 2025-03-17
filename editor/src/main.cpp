#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "Application.h"	
#include "Document.h"
#include "Panel.h"
#include "DisplayScale.h"
#include "Command.h"
#include "StandardActions.h"

#include "asset_types.h"
#include "AssetManager.h"

#include "TerrainDocument.h"
#include "TerrainMaterialDocument.h"
#include "TerrainInfoPanel.h"
#include "TerrainGenerationPanel.h"

#include "extras/IconsFontAwesome6.h"

using namespace EditorFramework;

class TerrainEditorApp : public Application
{
public:
protected:
    void OnRegisterPanels() override
    {
        RegisterPanel<TerrainInfoPanel>();
        RegisterPanel<TerrainGenerationPanel>();
    }

    void OnRegisterDocuments() override 
    {
        RegisterDocumentType<TerrainDocument>();
        RegisterDocumentType<TerrainMaterialDocument>();
    }

    void OnSetupInitalState() override
    { 
        OpenDocument<TerrainDocument>();
    }

	std::string_view GetWindowTitle() override
    { 
        return "Terrain Editor"; 
    }

    std::string_view GetApplicationFolderName() override
    {
        return "TerrainEditor";
    }
};

int main(int argc, char* argv[])
{
    Types::RegisterTypes(AssetSystem::AssetManager::TypeDB);
    TerrainEditorApp app;
    app.SetCommandLine(argv, argc);

    app.Run();

    return 0;
}