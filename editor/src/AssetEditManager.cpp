#include "AssetEditManager.h"



AssetEditEventRecord::AssetEditEventRecord(std::string_view name, size_t mergeID)
{

}

void AssetEditEventRecord::PushAction(const EditAction& action)
{

}


void AssetEditManager::RegisterEditCallbacks(Types::TypeValue* assetData)
{
    assetData->OnPrimitiveValueChanged.Add([this](const Types::PrimitiveValueChangedEvent& event)
        {
           // CurrentEditEvent.PushAction()
           // OnAssetDirty.Invoke()


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
