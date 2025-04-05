#include "AssetEditManager.h"



AssetEditEventRecord::AssetEditEventRecord(std::string_view name, size_t mergeID)
{

}

void AssetEditManager::RegisterEditCallbacks(Types::TypeValue* assetData)
{
    assetData->OnPrimitiveValueChanged.Add([this](const Types::PrimitiveValueChangedEvent& event)
        {
            OnAssetDirty.Invoke(AssetDirtyEvent{ event.Value->GetParent()->ID });

            auto* action = CurrentEditEvent->PushAction<AssetEditEventRecord::PrimitiveFieldEditAction>();
            action->AssetId = event.Value->GetParent()->ID;
            action->ValuePath = event.Path;
         //   action->PreviousValue = std::move(event);
        }, Token.GetToken());
}

void AssetEditManager::BeginEvent(std::string_view eventName, size_t mergeID /*= 0*/)
{
    FinalizeEvent();

    CurrentEditEvent->Name = eventName;
    CurrentEditEvent->MergeID = mergeID;
}

void AssetEditManager::FinalizeEvent()
{
    if (CurrentEditEvent && CurrentEditEvent->IsEmpty())
        return;

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
