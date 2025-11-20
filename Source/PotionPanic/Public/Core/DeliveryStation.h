#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/StationActor.h"
#include "DeliveryStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class USocketableComponent;
class AOrderClient;

UCLASS()
class ADeliveryStation : public AStationActor
{
	GENERATED_BODY()

public:
	ADeliveryStation();
	virtual void BeginPlay() override;

protected:

	UFUNCTION()
	void Deliver(USocketableComponent* OldHeld, USocketableComponent* NewHeld);

	AOrderClient* FindTargetClient() const;
};
