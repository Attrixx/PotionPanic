#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "AlembicStation.generated.h"

/**
 * Station for extracting essence.
 * Requires QTE interaction.
 */
UCLASS()
class STATIONS_API AAlembicStation : public AStationActorBase
{
	GENERATED_BODY()
	
protected:
	virtual void StartProcessing(const FInstruction& Instruction) override;

	// QTE Logic
	// TODO (Nath) : Implement QTE system properly

	virtual void Tick(float DeltaTime) override;
	virtual void FinishProcessing() override;



	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
