#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Instruction.h"
#include "StationDataAsset.generated.h"

class UActivityAsset;
class UStaticMesh;

/**
 * Configuration for a generic station.
 * Defines station visuals and executable instructions.
 */
UCLASS()
class STATIONS_API UStationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FText StationName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> StationMesh;

	/** Activities this station can perform (e.g. Cut, Boil). Used for validation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
	TArray<TObjectPtr<UActivityAsset>> SupportedActivities;

	/** List of instructions this station can execute. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
	TArray<FInstruction> Instructions;

	/** Legacy property kept for automatic migration of existing assets. */
	UPROPERTY()
	TArray<FInstruction> Recipes_DEPRECATED;
	// TODO (Nath): Remove Recipes_DEPRECATED after all StationDataAsset assets are resaved with Instructions.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float InteractionDistance = 200.0f;
};
