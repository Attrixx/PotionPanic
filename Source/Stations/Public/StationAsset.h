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
	
#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
