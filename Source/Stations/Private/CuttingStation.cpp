#include "CuttingStation.h"
#include "SocketComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

ACuttingStation::ACuttingStation()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemSocket = CreateDefaultSubobject<USocketComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(RootComponent);
}

void ACuttingStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTransforming && bRequiresProximityDuringCutting)
	{
		if (!CurrentPlayer || !IsPlayerInProximity(CurrentPlayer))
		{
			CancelTransformation();
			UE_LOG(LogTemp, Warning, TEXT("CuttingStation: Player left proximity, cutting cancelled"));
		}
	}
}

void ACuttingStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (bIsTransforming)
	{
		UE_LOG(LogTemp, Warning, TEXT("CuttingStation: Already cutting"));
		return;
	}

	StartTransformation(CuttingDuration);
	UE_LOG(LogTemp, Log, TEXT("CuttingStation: Started cutting"));
}

void ACuttingStation::OnTransformationCompleted()
{
	Super::OnTransformationCompleted();
	UE_LOG(LogTemp, Log, TEXT("CuttingStation: Cutting completed"));
}

void ACuttingStation::OnTransformationCancelled()
{
	Super::OnTransformationCancelled();
	UE_LOG(LogTemp, Warning, TEXT("CuttingStation: Cutting cancelled"));
}
