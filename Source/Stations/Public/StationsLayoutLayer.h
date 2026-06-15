// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "StationsLayoutLayer.generated.h"

class UStationAsset;

namespace GameTags
{
STATIONS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(StationSlot);
}

/**
 * Describes an ordered, incremental modification to a station layout.
 * 
 * A layer only defines the slots it explicitly changes. Unspecified slots
 * retain their value from the previously applied state.
 * 
 * APPLICATION ORDER MATTERS:
 *   Applying layers [A, B, C] may produce a different result than [A, C, B]
 *   if B and C write to overlapping slots. Last write wins on a given slot.
 *
 * Usage: layers are applied sequentially by the round system.
 *   The first layer in a sequence typically establishes the base layout.
 */
UCLASS()
class STATIONS_API UStationsLayoutLayer : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm="StationSlot", Categories="StationSlot"))
	UStationAsset* GetOverride(const FGameplayTag& StationSlot) const;
	
protected:

	/**
	 * Maps each affected slot to its new station.
	 * Slots absent from this map are left unchanged by this layer.
	 */
	UPROPERTY(EditAnywhere, Category = "Layout", meta = (Categories="StationSlot"))
	TMap<FGameplayTag, TObjectPtr<UStationAsset>> Overrides;

#if WITH_EDITOR

	// Harvest the StationSlot from each Station in the opened level and adds it in the overrides (if not already added).
	UFUNCTION(CallInEditor, Category = "Editor Tools", DisplayName = "Harvest Slots")
	void HarvestTagsFromCurrentLevel();

	// Apply the overrides in the opened level. You can apply multiple layers on top of each other using this button.
	// WARNING: Do not save the level during preview, or it will overwrite the default layout. Press Ctrl+Z to undo the preview.
	UFUNCTION(CallInEditor, Category = "Editor Tools", DisplayName = "Preview Layer")
	void PreviewLayerInLevel();
	
#endif
};
