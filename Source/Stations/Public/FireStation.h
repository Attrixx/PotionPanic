#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "FireStation.generated.h"

class APlayerController;
class AFireStation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireStationProcessRequestedEvent, APlayerController*, InstigatorController, AFireStation*, FireStation);

/**
 * Station dedicated to cauldron processing.
 * Accepts only the configured cauldron item id.
 */
UCLASS()
class STATIONS_API AFireStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	AFireStation();

	virtual void Interact(APlayerController& InInstigator) override;
	virtual bool CanPlaceItem(const FPrimaryAssetId& ItemId) const override;

	UPROPERTY(BlueprintAssignable, Category = "FireStation")
	FFireStationProcessRequestedEvent OnProcessRequested;

protected:
	/** Item id representing the cauldron that can be placed on this station. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireStation")
	FPrimaryAssetId RequiredCauldronItemId;
};
