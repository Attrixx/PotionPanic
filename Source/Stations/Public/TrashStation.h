#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "TrashStation.generated.h"

/**
 * Disposal station.
 * Only ingredient items can be trashed.
 */
UCLASS()
class STATIONS_API ATrashStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	virtual void Interact(APlayerController& InInstigator) override;
	virtual bool CanPlaceItem(const FPrimaryAssetId& ItemId) const override;

	// TODO (Nath): Add dedicated VFX/SFX feedback for successful ingredient disposal.
};
