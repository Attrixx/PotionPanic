#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "CuttingStation.generated.h"

class USocketComponent;
class AItemActor;

/**
 * Cutting Station - Transforms items via timed cutting process.
 * 
 * Special behavior: Requires player to stay within proximity during cutting.
 * If player moves away, the cutting process is cancelled automatically.
 * This simulates needing to actively operate the guillotine.
 */
UCLASS()
class STATIONS_API ACuttingStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	ACuttingStation();
	virtual void Execute(const FInstruction& Instruction) override;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void OnTransformationCompleted() override;
	virtual void OnTransformationCancelled() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USocketComponent> ItemSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cutting")
	float CuttingDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cutting")
	bool bRequiresProximityDuringCutting = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<AItemActor> CurrentItem;
};
