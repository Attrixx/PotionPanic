#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationActor.generated.h"

class UStationAsset;
class UHolderComponent;
class UActivityStep;
class UItemAsset;

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

	UFUNCTION()
	void Interact(AActor* InInstigator) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetStationAsset(UStationAsset* NewStationAsset);

private: // Activity logic

	UFUNCTION()
	void OnActivityFinished(const FActivityOutput& ActivityOutput);

	void ResetCurrentActivities();
	void ExecuteNextActivity(AActor* Instigator);

private:

	UFUNCTION()
	void OnRep_StationAsset();

	void ApplyStationAsset();

private:

	UPROPERTY(EditAnywhere, Category = "Station|Data", ReplicatedUsing=OnRep_StationAsset)
	TObjectPtr<UStationAsset> StationAsset;

	UPROPERTY(VisibleAnywhere, Category = "Station|Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Station|Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	EStationStatus Status = EStationStatus::Idle;

	UPROPERTY()
	TArray<UActivityStep*> CachedActivitySteps;
	int32 ActivityIndex = -1;

	UPROPERTY()
	TObjectPtr<UItemAsset> ActivityOutputItem;
};
