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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispenser")
	FPrimaryAssetId ItemToDispense;

	/** Optional actor class override for spawned item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispenser")
	TSubclassOf<AItemActor> ItemActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispenser", meta = (ClampMin = "0.0"))
	float DispenseCooldownSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dispenser", meta = (AllowPrivateAccess = "true"))
	float NextAllowedDispenseTimeSeconds = 0.0f;
};
