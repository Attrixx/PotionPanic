#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Instruction.h"
#include "StationDataAsset.generated.h"

class UActivityAsset;
class UStaticMesh;

/**
 * Configuration for a generic station.
 * Defines station visuals/capabilities.
 */
UCLASS()
class STATIONS_API UStationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	FText StationName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> StationMesh;

	/** Activities this station can perform (e.g. Cut, Boil). Used for validation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Logic")
	TArray<TObjectPtr<UActivityAsset>> SupportedActivities;

	/** Optional reference instructions for external orchestrators/tools. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Logic")
	TArray<FInstruction> Instructions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	float InteractionDistance = 200.0f;
};
