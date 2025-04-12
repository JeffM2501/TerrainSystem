#include "AssetEditManager.h"

AssetEditManager* AssetEditManager::CurrentEditManager = nullptr;


AssetEditEventRecord::AssetEditEventRecord(std::string_view name, size_t mergeID)
{

}

void AssetEditEventRecord::CheckMergeability()
{
    if (!AllowMerge || Actions.empty())
        return;

    size_t lastAssetId = 0;

    CommonAncestorPath.Elements.clear();

    FieldName.clear();

    for (auto& action : Actions)
    {
        if (lastAssetId == 0)
        {
            lastAssetId = action->AssetId;
            CommonAncestorPath = action->ValuePath;
        }
        else
        {
            if (lastAssetId != action->AssetId)
            {
                AllowMerge = false;
                return;
            }

            if (CommonAncestorPath != action->ValuePath)
            {
                AllowMerge = false;
                return;
            }
        }
    }
}

void AssetEditManager::RegisterEditCallbacks(Types::TypeValue* assetData)
{
    assetData->OnPrimitiveValueChanged.Add([this](const Types::ValueChangedEvent& event)
        {
            OnAssetDirty.Invoke(AssetDirtyEvent{ event.Value->GetParent()->ID });

            auto* action = CurrentEditEvent->PushAction<AssetEditEventRecord::PrimitiveFieldEditAction>();
            action->AssetId = event.Value->GetParent()->ID;
            action->ValuePath = event.Path;
            action->ValueRecord = event.Record;
        }, Token.GetToken());
}

void AssetEditManager::BeginEvent(std::string_view eventName, bool allowMerge)
{
    FinalizeEvent();

    if (!CurrentEditEvent)
        CurrentEditEvent = std::make_unique<AssetEditEventRecord>("", 0);

    CurrentEditEvent->Name = eventName;
    CurrentEditEvent->AllowMerge = allowMerge;
}

void AssetEditManager::FinalizeEvent()
{
    if (!CurrentEditEvent || CurrentEditEvent->IsEmpty())
        return;

    CurrentEditEvent->CheckMergeability();

    if (!CurrentEditEvent->IsEmpty() && CurrentEditEvent->AllowMerge)
    {
        CurrentEditEvent->FieldName = OpenAssets[CurrentEditEvent->Actions[0]->AssetId]->GetFieldNameFromPath(CurrentEditEvent->CommonAncestorPath);
    }

    if (!EditEvents.empty())
    {
        if (CurrentEditEvent->AllowMerge && EditEvents.back()->AllowMerge)
        {
            if (!CurrentEditEvent->CommonAncestorPath.Elements.empty() && CurrentEditEvent->CommonAncestorPath == EditEvents.back()->CommonAncestorPath)
            {
                EditEvents.back()->Actions.insert(EditEvents.back()->Actions.end(),
                    std::make_move_iterator(CurrentEditEvent->Actions.begin()),
                    std::make_move_iterator(CurrentEditEvent->Actions.end()));
                CurrentEditEvent->Actions.clear();
                return;
            }
        }
    }

    EditEvents.push_back(std::move(CurrentEditEvent));
    CurrentEditEvent = std::make_unique<AssetEditEventRecord>("", 0);
}

bool AssetEditManager::CanUndo() const
{
    return !EditEvents.empty() && CurrentEditEventIndex > 0;
}

bool AssetEditManager::CanRedo() const
{
    return !EditEvents.empty() && CurrentEditEventIndex < EditEvents.size();
}

void AssetEditManager::Undo()
{

}

void AssetEditManager::Redo()
{

}
