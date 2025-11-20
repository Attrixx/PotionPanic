#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CauldronStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class UStationComponent;
class USpawnerComponent;

UCLASS()
class ACauldronStation : public AActor
{
	GENERATED_BODY()

public:
	ACauldronStation();

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
	TObjectPtr<USpawnerComponent> SpawnerComponent;

private:
	void Store(class USocketableComponent* OldHeld, class USocketableComponent* NewHeld);
};