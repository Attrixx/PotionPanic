#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DispenserStation.generated.h"

class AItemActor;

/**
 * Infinite source station for one configured item.
 * Does not run instruction processing.
 */
UCLASS()
class STATIONS_API ADispenserStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	virtual void Interact(APlayerController& InInstigator) override;

protected:
	/** Asset id to spawn when a player with empty hands interacts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dispenser")
	FPrimaryAssetId ItemToDispense;

	/** Optional actor class override for spawned item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dispenser")
	TSubclassOf<AItemActor> ItemActorClass = nullptr;

	// TODO (Nath): Add per-dispenser cooldown if needed by level balance.
};
