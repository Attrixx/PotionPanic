#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DeliveryStation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeliveryStationItemDeliveredDelegate, FPrimaryAssetId, DeliveredItemId, AActor*, SourceStation);

/**
 * Delivery station for completed items.
 * Consumes held item and emits a delivery event.
 */
UCLASS()
class STATIONS_API ADeliveryStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	virtual void Interact(APlayerController& InInstigator) override;
	virtual bool CanPlaceItem(const FPrimaryAssetId& ItemId) const override;

	UPROPERTY(BlueprintAssignable, Category = "Delivery")
	FDeliveryStationItemDeliveredDelegate OnItemDelivered;

protected:
	/** Optional allow-list. If empty, any valid item id is accepted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delivery")
	TArray<FPrimaryAssetId> AcceptedItems;
};
