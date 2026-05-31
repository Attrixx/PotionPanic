// Fill out your copyright notice in the Description page of Project Settings.

#include "StationsLayoutLayer.h"

#include "StationAsset.h"


namespace GameTags
{
UE_DEFINE_GAMEPLAY_TAG_COMMENT(StationSlot,
	"Station Slot",
	"Parent tag for all Station Slots. Used to identify stations in layouts.");
}

UStationAsset* UStationsLayoutLayer::GetOverride(const FGameplayTag& StationSlot) const
{
	if (auto* StationAsset = Overrides.Find(StationSlot))
	{
		return *StationAsset;
	}
	return nullptr;
}

#if WITH_EDITOR
#include "Editor.h"
#include "EngineUtils.h"
#include "StationActor.h"
#include "Misc/MessageDialog.h"

void UStationsLayoutLayer::HarvestTagsFromCurrentLevel()
{
	if (!GEditor) return;

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) return;

	GEditor->BeginTransaction(FText::FromString("Harvest Station Slots"));

	for (TActorIterator<AStationActor> It(EditorWorld); It; ++It)
	{
		AStationActor* Station = *It;
		if (Station && Station->GetStationSlot().IsValid())
		{
			if (!Overrides.Contains(Station->GetStationSlot()))
			{
				Modify();
				Overrides.Add(Station->GetStationSlot(), nullptr);
			}
		}
	}

	GEditor->EndTransaction();
}

void UStationsLayoutLayer::PreviewLayerInLevel()
{
	if (!GEditor) return;

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) return;

	// Ctrl+Z setup
	GEditor->BeginTransaction(FText::FromString("Preview Station Layer"));

	for (TActorIterator<AStationActor> It(EditorWorld); It; ++It)
	{
		AStationActor* Station = *It;
		if (!Station) continue;

		if (TObjectPtr<UStationAsset>* NewAssetPtr = Overrides.Find(Station->GetStationSlot()))
		{
			if (*NewAssetPtr != nullptr && *NewAssetPtr != Station->GetStationAsset())
			{
				Station->Modify();
				Station->SetStationAsset(*NewAssetPtr);
			}
		}
	}

	// Register Ctrl+Z modifications
	GEditor->EndTransaction();
}

#endif // WITH_EDITOR
