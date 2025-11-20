#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class USpawnerComponent;

UCLASS()
class ADeliveryStation : public AActor
{
	GENERATED_BODY()

public:
	ADeliveryStation();

protected:
	virtual void BeginPlay() override;

private:
	void Deliver(class USocketableComponent* OldHeld, class USocketableComponent* NewHeld);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USocketComponent> SocketComponent;
};