#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DeliveryStation.generated.h"

/**
 * Station where players deliver finished potions.
 * Validates the item and awards score.
 */
UCLASS()
class STATIONS_API ADeliveryStation : public AStationActorBase
{
	GENERATED_BODY()
	
public:
	ADeliveryStation();

	virtual bool CanPlaceItem(const UItemAsset* Item) const override;
	virtual void Interact(APlayerController& InInstigator) override;

protected:
	virtual void StartProcessing(const FInstruction& Instruction) override;
	virtual void FinishProcessing() override;
};
