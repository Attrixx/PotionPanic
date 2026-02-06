#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DispenserStation.generated.h"

class UItemAsset;

/**
 * Station that dispenses infinite raw ingredients.
 * Player must have empty hands to interact.
 */
UCLASS()
class STATIONS_API ADispenserStation : public AStationActorBase
{
	GENERATED_BODY()
	
public:
	ADispenserStation();

	virtual void Interact(APlayerController& InInstigator) override;

protected:
	/** The item to dispense when interacted with. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	FPrimaryAssetId IngredientToDispense;
};
