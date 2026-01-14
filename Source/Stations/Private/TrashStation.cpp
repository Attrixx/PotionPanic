#include "TrashStation.h"
#include "ItemActor.h"

ATrashStation::ATrashStation()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATrashStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);
	UE_LOG(LogTemp, Log, TEXT("TrashStation: Destroying item"));
}
