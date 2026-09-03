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
	 * If false the station's holder is a pass-through, never a storage spot: an item left on it
	 * with nothing to do is handed back to the instigator, or ejected if that fails.
	 * Set to false for stations that only consume, like a delivery counter or a bin.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanStoreItems = true;

	/**
	 * Whether the holder catches items that begin overlapping it, rather than only receiving them
	 * from a deliberate interaction. Set to false for a station players must hand items to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanCatchItems = true;

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
