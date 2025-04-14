#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rlImGui.h"
#include "imgui_utils.h"

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
#include "PropertiesPanel.h"
#include "HistoryPanel.h"

#include "extras/IconsFontAwesome6.h"
#include "tinyfiledialogs.h"

using namespace EditorFramework;

class SetupAssetRootDialog : public Dialog
{
protected:
    DialogResult OnShow() override
    {
        if (ImGui::BeginTable("SETUPTABLE", 2, ImGuiTableFlags_SizingStretchProp, ImGui::GetContentRegionAvail()))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            rlImGuiPushIconFont();
            ImGui::Dummy(ImVec2{ ScaleToDPI(10.0f), ScaleToDPI(5.0f) });
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_NavHighlight], ICON_FA_HARD_DRIVE);
            ImGui::PopFont();
            ImGui::SameLine(0, 0);
            ImGui::Dummy(ImVec2{ ScaleToDPI(10.0f), 1 });

            ImGui::TableNextColumn();

            ImGui::TextUnformatted("Please select the asset root folder");

            auto buttonSize = ImGuiUtils::GetButtonsSize(ICON_FA_ELLIPSIS, "", "");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonSize.x - ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::InputText("###PATH", AssetRootPath, sizeof(AssetRootPath), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                return DialogResult::Accept;
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ELLIPSIS))
            {
                const char* result = tinyfd_selectFolderDialog("Asset Root", AssetRootPath);
                if (result != nullptr)
                {
                    memcpy(AssetRootPath, result, std::min(strlen(result), sizeof(AssetRootPath) - 1));
                }
            }
            ImGui::EndTable();
        }
        return DialogResult::None;
    }

public:
    char AssetRootPath[512] = { 0 };
    SetupAssetRootDialog(std::string_view assetRoot, DialogCallback onAccept)
    {
        if (assetRoot.empty())
            memcpy(AssetRootPath, assetRoot.data(), std::min(assetRoot.size(), sizeof(AssetRootPath) - 1));

        DialogName = "Setup Asset Root";
        AcceptCallback = onAccept;
        AcceptName = DialogOK;
        MinimumSize = { ScaleToDPI(350.0f), ScaleToDPI(100.0f) };
    }
};

class TerrainEditorApp : public Application
{
public:
protected:
    void OnRegisterPanels() override
    {
        RegisterPanel<PropertiesPanel>();
        RegisterPanel<TerrainInfoPanel>();
        RegisterPanel<TerrainGenerationPanel>();
        RegisterPanel<HistoryPanel>();
    }

    void OnRegisterDocuments() override
    {
        RegisterDocumentType<TerrainDocument>();
        RegisterDocumentType<TerrainMaterialDocument>();
    }

    void OnSetupInitalState() override
    {
        OpenDocument<TerrainMaterialDocument>();

        if (AssetRoot.empty())
        {
            ShowDialogBox<SetupAssetRootDialog>(AssetRoot, [this](Dialog& dlog)
                {
                    auto* setup = static_cast<SetupAssetRootDialog*>(&dlog);
                    AssetSystem::AssetManager::SetAssetRoot(setup->AssetRootPath);
                    AssetRoot = setup->AssetRootPath;
                });
        }
        else
        {
            AssetSystem::AssetManager::SetAssetRoot(AssetRoot);
        }
    }

    std::string_view GetWindowTitle() override
    {
        return "Terrain Editor";
    }

    std::string_view GetApplicationFolderName() override
    {
        return "TerrainEditor";
    }

    void OnLoadSettings(rapidjson::Document& doc) override
    {
        if (doc.HasMember("AssetRoot"))
            AssetRoot = doc.FindMember("AssetRoot")->value.GetString();

        Application::OnLoadSettings(doc);
    }

    void OnSaveSettings(rapidjson::Document& doc) override
    {
        if (!AssetRoot.empty())
            doc.AddMember("AssetRoot", rapidjson::StringRef(AssetRoot.c_str(), AssetRoot.size()), doc.GetAllocator());

        Application::OnSaveSettings(doc);
    }

private:
    std::string AssetRoot;
};

int main(int argc, char* argv[])
{
    Types::RegisterTypes();
    TerrainEditorApp app;
    app.SetCommandLine(argv, argc);

    app.Run();

    return 0;
}