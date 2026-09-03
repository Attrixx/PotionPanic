#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "StationAsset.generated.h"

class AStationVisualActor;

/**
 * UStationAsset — Data definition for a station.
 *
 * Carries everything that makes one station distinct from another.
 * AStationActor has no subclasses; all customization goes here.
 */
UCLASS()
class STATIONS_API UStationAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText StationName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AStationVisualActor> VisualActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Activity"))
	FGameplayTagContainer ImplementedActivities;
	
	/** If false the station will never take from the instigator, even if the activity allows it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanEverTakeItemFromInstigator = true;

	/**
	 * If false the station's holder is a pass-through, never a storage spot: it won't catch
	 * overlapping items, and any item still on it once an activity concludes is handed back
	 * to the instigator, or ejected if that fails.
	 * Set to false for stations that only consume, like a delivery counter or a bin.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanStoreItems = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> OnInteractSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> OnActivityGoingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> OnActivitySuccessSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> OnActivityFailedSound;

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
