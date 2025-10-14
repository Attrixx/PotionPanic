#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawner.generated.h"

class ASpawnableObject;

UCLASS()
class AObjectSpawner : public AActor
{
	GENERATED_BODY()

public:
	AObjectSpawner();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnObject();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* SpawnerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ExposeOnSpawn = true))
	TSubclassOf<ASpawnableObject> ObjectToSpawn;



};