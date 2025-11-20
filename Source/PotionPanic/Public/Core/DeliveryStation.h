#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class USocketableComponent;
class AOrderClient;

UCLASS()
class POTIONPANIC_API ADeliveryStation : public AActor
{
	GENERATED_BODY()

public:
	ADeliveryStation();
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USocketComponent* SocketComponent;

	UFUNCTION()
	void Deliver(USocketableComponent* OldHeld, USocketableComponent* NewHeld);

	AOrderClient* FindTargetClient() const;
};
