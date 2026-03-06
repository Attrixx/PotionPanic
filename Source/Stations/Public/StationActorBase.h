#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Recipes/Public/RecipeSystem.h"
#include "StationActorBase.generated.h"

class AItemActor;
class APlayerController;
class UHolderComponent;
class UActivityStep;
class UItemAsset;
class UStationAsset;

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
class STATIONS_API AStationActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AStationActorBase();
	void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void Interact(AActor* InInstigator) override;

private:
	void BeginPlay() override;
	
	void SetStationAsset(UStationAsset* NewAsset);
	UFUNCTION()
	void OnActivityFinished(const FActivityOutput& ActivityOutput);
	
	void ResetCurrentActivities();
	void ExecuteNextActivity(AActor* Instigator);

private:
	UPROPERTY(EditAnywhere, Category = "Station|Data")
	TObjectPtr<UStationAsset> StationAsset;
	
	UPROPERTY(EditAnywhere, Category = "Station|Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	UPROPERTY(EditAnywhere, Category = "Station|Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	EStationStatus Status = EStationStatus::Idle;
	UPROPERTY()
	TArray<UActivityStep*> CachedActivitySteps;
	int32 ActivityIndex = -1;
	
	UPROPERTY()
	TObjectPtr<UItemAsset> ActivityOutputItem;
};
