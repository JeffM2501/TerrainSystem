#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "Application.h"	
#include "Document.h"
#include "Panel.h"
#include "DisplayScale.h"
#include "Command.h"
#include "StandardActions.h"

#include "TerrainDocument.h"
#include "TerrainInfoPanel.h"
#include "TerrainGenerationPanel.h"

#include "extras/IconsFontAwesome6.h"

using namespace EditorFramework;

class TerrainEditorApp : public SingleDocumentTypeApp<TerrainDocument>
{
public:
protected:
    void OnRegisterPanels() override
    {
        RegisterPanel<TerrainInfoPanel>();
        RegisterPanel<TerrainGenerationPanel>();
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
    TerrainEditorApp app;
    app.SetCommandLine(argv, argc);

    app.Run();

    return 0;
}