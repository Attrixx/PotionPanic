#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "StationAsset.generated.h"

class UStationVisualProvider;

/**
 * UStationAsset — Data definition for a station.
 *
 * Carries everything that makes one station distinct from another.
 * AStationActor has no subclasses; all customization goes here.
 *
 * VisualProvider   — Strategy object built by the designer. Defines how the
 *                    station looks (mesh, skeletal, child actor, etc.)
 *                    without constraining AStationActor.
 *
 * ImplementedActivities — Tags declaring which activity types this station
 *                         supports. Queried by the instruction system at runtime.
 */
UCLASS()
class STATIONS_API UStationAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText StationName;

	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UStationVisualProvider> VisualProvider;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Activity"))
	FGameplayTagContainer ImplementedActivities;
};
