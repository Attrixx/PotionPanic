#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CutStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;
class UStationComponent;
class USpawnerComponent;

UCLASS()
class ACutStation : public AActor
{
	GENERATED_BODY()

public:
	ACutStation();

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
	void RemoveOnSocket(APawn* InInstigator, TSubclassOf<AActor> Item);
};