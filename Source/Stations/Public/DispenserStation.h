#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DispenserStation.generated.h"

class UItemAsset;
class AItemActor;

/**
 * Dispenser Station - Spawns items as configured in Instruction.OutputItem.
 * Acts as an infinite (or limited) source of ingredients for recipes.
 * Supports stock management with optional depletion.
 */
UCLASS()
class STATIONS_API ADispenserStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ADispenserStation();
	virtual void Execute(const FInstruction& Instruction) override;

private:
	AItemActor* SpawnItemFromAsset(const FPrimaryAssetId& AssetId);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	FVector SpawnOffset = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dispenser")
	TSubclassOf<AItemActor> ItemActorClass;

	// Stock management: set false to enable limited supply mode
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dispenser")
	bool bUnlimitedSupply = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	int32 CurrentStock = 0;
};
