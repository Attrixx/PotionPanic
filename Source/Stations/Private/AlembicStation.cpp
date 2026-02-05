#include "AlembicStation.h"
#include "Net/UnrealNetwork.h"

void AAlembicStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(AAlembicStation, QTEState);
}

void AAlembicStation::StartProcessing(const FInstruction& Instruction)
{
    // TODO (Nath): Initialize QTE Widget here
    Super::StartProcessing(Instruction);
    
    // TODO: QTEState initialization removed in QTE cleanup
}



void AAlembicStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAlembicStation::FinishProcessing()
{
    // TODO: Re-verify QTEState here when implemented
    Super::FinishProcessing();
}
