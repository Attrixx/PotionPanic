#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationActor.generated.h"

class AItemActor;
class UStationAsset;
class UHolderComponent;
class UActivityExecutor;

/**
 * Base class for all station actors.
 * 
 * Stations are interactable actors that support specific activities.
 * All station logic is handled through the Interact() method and an external manager.
 */
UCLASS()
class STATIONS_API AStationActor : public AActor, public IInteractable
{
	GENERATED_BODY()

	AStationActor();
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;

	void Interact_Implementation(AActor* InInstigator) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetStationAsset(UStationAsset* NewStationAsset);

private:

	void FetchInstructions(AActor* InInstigator);

	UFUNCTION()
	void Holder_OnCarriableChanged(UHolderComponent* Holder);

	UFUNCTION()
	void OnRep_StationAsset();

	void ApplyStationAsset();

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemActor> ItemClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_StationAsset)
	TObjectPtr<UStationAsset> StationAsset;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UActivityExecutor> Executor;
};
