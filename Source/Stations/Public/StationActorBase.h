#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationAsset.h"
#include "Interactions/Public/InteractionBase.h"
#include "StationActorBase.generated.h"

class AItemActor;
class APlayerController;
class UHolderComponent;
class UInteractionBase;
class UItemAsset;

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
	void Interact(APlayerController* InInstigator) override;

private:
	virtual void BeginPlay() override;
	
	void SetStationAsset(UStationAsset* NewAsset);
	UFUNCTION()
	void OnInteractionFinished(FInteractionOutput InteractionOutput);
	
	void ResetCurrentInteractions();
	void ExecuteNextInteraction();

private:
	UPROPERTY(EditAnywhere, Category = "Station|Data")
	TObjectPtr<UStationAsset> StationAsset;
	
	UPROPERTY(EditAnywhere, Category = "Station|Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	UPROPERTY(EditAnywhere, Category = "Station|Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	
	UPROPERTY()
	APlayerController* CachedInstigator;
	UPROPERTY()
	TArray<UInteractionBase*> CachedInteractions;
	int32 InteractionIndex = -1;
	
	UPROPERTY()
	TObjectPtr<UItemAsset> InteractionOutputItem;
};
