#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Instruction.h"
#include "StationDataAsset.generated.h"

class UActivityAsset;
class UStaticMesh;

/**
 * Configuration for a generic station.
 * Defines the visuals, recipes, and capabilities of a station.
 */
UCLASS()
class STATIONS_API UStationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FText StationName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> StationMesh;

	/** Activities this station can perform (e.g. Cut, Boil). Used for validation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
	TArray<UActivityAsset*> SupportedActivities;

	/** List of recipes/instructions this station can execute. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
	TArray<FInstruction> Recipes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float InteractionDistance = 200.0f;
};
