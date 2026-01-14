#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "AlambicStation.generated.h"

class USocketComponent;
class AItemActor;

/**
 * Alambic Station - Extracts essence from ingredients via timed distillation.
 * Transforms raw ingredients into refined essences for potion crafting.
 */
UCLASS()
class STATIONS_API AAlambicStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	AAlambicStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	virtual void OnTransformationCompleted() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> ItemSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extraction")
	float DefaultExtractionDuration = 4.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<AItemActor> CurrentIngredient;
};
