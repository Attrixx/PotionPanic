#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "TrashStation.generated.h"

/**
 * Station that destroys any item placed on it.
 * Duration should be 0 (Instant).
 * Output should be null.
 */
UCLASS()
class STATIONS_API ATrashStation : public AStationActorBase
{
	GENERATED_BODY()
	
public:
    virtual bool CanPlaceItem(const UItemAsset* Item) const override;
    // TODO (Nath): Add FX/Sound on destroy? override OnProcessingFinished to play sound before destroy.
};
