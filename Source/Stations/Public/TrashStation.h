#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "TrashStation.generated.h"

class AItemActor;

/**
 * Trash Station - Immediately destroys any item placed in it.
 * Used for discarding unwanted ingredients or mistakes.
 */
UCLASS()
class STATIONS_API ATrashStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ATrashStation();
	virtual void Execute(const FInstruction& Instruction) override;
};
