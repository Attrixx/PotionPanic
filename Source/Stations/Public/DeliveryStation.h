#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "DeliveryStation.generated.h"

class USocketComponent;
class AItemActor;

/**
 * Delivery Station - Accepts completed potions for level progression.
 * 
 * Validates delivered items against active orders and broadcasts events
 * for game systems to handle scoring and level completion.
 * Essential for gameplay loop completion.
 */
UCLASS()
class STATIONS_API ADeliveryStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ADeliveryStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> ItemSocket;

	// Events for game integration
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeliveryCompleted, AItemActor*, DeliveredItem);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeliveryCompleted OnDeliveryCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeliveryFailed, AItemActor*, RejectedItem);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeliveryFailed OnDeliveryFailed;
};
