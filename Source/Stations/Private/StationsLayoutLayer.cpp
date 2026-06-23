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
#include "StationActor.h"
#include <Editor.h>
#include <EngineUtils.h>
#include <Misc/DataValidation.h>

EDataValidationResult UStationsLayoutLayer::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	int32 NullOverridesNum = 0;
	for (auto& [Slot, Asset] : Overrides)
	{
		if (!IsValid(Asset))
		{
			++NullOverridesNum;
			Context.AddWarning(FText::FromString(FString::Format(
				TEXT("Slot '{0}' has an invalid asset override. It will be ignored when applied."),
				{Slot.GetTagName().ToString()})));
		}
	}

	if (NullOverridesNum > 0)
	{
		Context.AddWarning(FText::FromString(FString::Format(
			TEXT("Tip: Use the 'Clean Overrides' button to fix the above {0} warnings."),
			{NullOverridesNum})));
	}

	return Result;
}

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
				Overrides.Add(Station->GetStationSlot(), Station->GetStationAsset());
			}
		}
	}

	GEditor->EndTransaction();
}

void UStationsLayoutLayer::RemoveNullOverrides()
{
	if (!GEditor) return;

	GEditor->BeginTransaction(FText::FromString("Remove Null Overrides"));

	Modify();
	for (auto It = Overrides.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Value))
		{
			It.RemoveCurrent();
		}
	}

	GEditor->EndTransaction();
}

void UStationsLayoutLayer::PreviewLayerInLevel()
{
	if (!GEditor) return;

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) return;

	GEditor->BeginTransaction(FText::FromString("Preview Station Layer"));

	for (TActorIterator<AStationActor> It(EditorWorld); It; ++It)
	{
		AStationActor* Station = *It;
		if (!Station) continue;

		if (UStationAsset* NewAsset = GetOverride(Station->GetStationSlot()))
		{
			if (NewAsset != Station->GetStationAsset())
			{
				Station->Modify();
				Station->SetStationAsset(NewAsset);
			}
		}
	}

	GEditor->EndTransaction();
}

#endif // WITH_EDITOR
