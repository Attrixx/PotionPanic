#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "TrashStation.generated.h"

class AItemActor;
class USocketComponent;

/**
 * Trash Station - Destroys items or empties containers placed in it.
 * Used for discarding unwanted ingredients or mistakes.
 * 
 * Behavior:
 * - Simple items (ingredients): Destroyed entirely
 * - Containers (cauldron, etc.): Only contents are destroyed, container is kept
 * 
 * Items placed in the ItemSocket will be processed when Execute is called.
 */
UCLASS()
class STATIONS_API ATrashStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ATrashStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> ItemSocket;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<AItemActor> CurrentItem;
};
