#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "CuttingStation.generated.h"

/**
 * Station for cutting ingredients.
 * Requires player to remain close (Proximity Check).
 */
UCLASS()
class STATIONS_API ACuttingStation : public AStationActorBase
{
	GENERATED_BODY()
	
public:
    ACuttingStation();
};
