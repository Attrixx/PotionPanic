#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ActivityAsset.h"
#include "Instruction.h"
#include "StationActorBase.generated.h"

class AItemActor;
class APlayerController;
class UHolderComponent;

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

	UFUNCTION()
	void Interact(APlayerController* InInstigator) override;

	UFUNCTION(BlueprintCallable, Category = "Station")
	void SetInstruction(const FInstruction& InInstruction) { CurrentInstruction = InInstruction; }

	const TArray<UActivityAsset*>& GetActivities() const { return Activities; }

protected:
	virtual void BeginPlay() override;

protected:
	// TODO(Nath): Configure socket for item placement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	// Configure in Blueprint: which activities this station can perform
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<UActivityAsset*> Activities;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	FInstruction CurrentInstruction;
};
