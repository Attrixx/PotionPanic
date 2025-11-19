#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class UStationComponent;
class USpawnerComponent;
class UDeliveryComponent;

UCLASS()
class ADeliveryStation : public AActor
{
	GENERATED_BODY()

public:
	ADeliveryStation();

protected:
	virtual void BeginPlay() override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USocketComponent> SocketComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStationComponent> StationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDeliveryComponent> DeliveryComponent;
};