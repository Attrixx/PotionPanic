#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationActor.generated.h"

class AItemActor;
class UStationAsset;
class UHolderComponent;
class UActivityStep;
class UItemAsset;
struct FActivityOutput;

UENUM()
enum class EStationStatus : uint8
{
	Idle = 0,
	Ready,
	Busy
};

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

	void Interact(AActor* InInstigator) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetStationAsset(UStationAsset* NewStationAsset);

private: // Activity logic

	void OnActivityFinished(const FActivityOutput& ActivityOutput);
	void ResetCurrentActivities();
	void ExecuteNextActivity();

private:

	UFUNCTION()
	void OnRep_StationAsset();

	void ApplyStationAsset();

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemActor> ItemClass;

	UPROPERTY(EditAnywhere, Category = "Station|Data", ReplicatedUsing=OnRep_StationAsset)
	TObjectPtr<UStationAsset> StationAsset;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	EStationStatus Status = EStationStatus::Idle;

	TWeakObjectPtr<AActor> LastInstigator;
	
	UPROPERTY()
	TArray<UActivityStep*> CachedActivitySteps;
	int32 ActivityIndex = -1;

	UPROPERTY()
	TObjectPtr<UItemAsset> ActivityOutputItem;
};
