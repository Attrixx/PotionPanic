#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "WoodFireStation.generated.h"

/**
 * Specialized station for Cooking.
 * Only accepts a Cauldron item.
 */
UCLASS()
class STATIONS_API AWoodFireStation : public AStationActorBase
{
	GENERATED_BODY()
	

public:
	virtual bool CanPlaceItem(const UItemAsset* Item) const override;

protected:
	virtual void StartProcessing(const FInstruction& Instruction) override;
	virtual void FinishProcessing() override;

protected:
	// TODO (Nath): Set this to the Cauldron ItemAsset in Blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Station Filter")
	TObjectPtr<UItemAsset> RequiredItem;
};
