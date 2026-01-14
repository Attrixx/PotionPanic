#include "AlambicStation.h"
#include "SocketComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

AAlambicStation::AAlambicStation()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemSocket = CreateDefaultSubobject<USocketComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(RootComponent);
}

void AAlambicStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (bIsTransforming)
	{
		UE_LOG(LogTemp, Warning, TEXT("AlambicStation: Already extracting"));
		return;
	}

	StartTransformation(DefaultExtractionDuration);
	UE_LOG(LogTemp, Log, TEXT("AlambicStation: Started extraction, duration: %f"), DefaultExtractionDuration);
}

void AAlambicStation::OnTransformationCompleted()
{
	Super::OnTransformationCompleted();
	UE_LOG(LogTemp, Log, TEXT("AlambicStation: Extraction completed"));
}
