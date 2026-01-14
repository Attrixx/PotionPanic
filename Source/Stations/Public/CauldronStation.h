#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "CauldronStation.generated.h"

class USocketComponent;
class AItemActor;

/**
 * Cauldron Station - Combines multiple ingredients through cooking.
 * 
 * Supports multi-ingredient recipes: Execute() can be called multiple times
 * to add ingredients before cooking starts. Once max ingredients or trigger
 * condition is met, begins timed cooking transformation.
 */
UCLASS()
class STATIONS_API ACauldronStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ACauldronStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	virtual void OnTransformationCompleted() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> CauldronSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooking")
	float CookingDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cauldron")
	int32 MaxIngredients = 5;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TArray<TObjectPtr<AItemActor>> Ingredients;
};
