#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "FirecampStation.generated.h"

class USocketComponent;
class AItemActor;

/**
 * Firecamp Station - Exclusive cooking station for cauldron-items.
 * 
 * The firecamp accepts a cauldron-item on its socket and processes
 * the ingredients contained within the cauldron.
 */
UCLASS()
class STATIONS_API AFirecampStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	AFirecampStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	virtual void OnTransformationCompleted() override;

	// Helper to check if cauldron has ingredients
	bool CauldronHasIngredients() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> CauldronSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooking")
	float CookingDuration = 5.0f;

	// Runtime state
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<AItemActor> CurrentCauldron;
};
