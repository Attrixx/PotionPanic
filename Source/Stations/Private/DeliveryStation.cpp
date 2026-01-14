#include "DeliveryStation.h"
#include "SocketComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

ADeliveryStation::ADeliveryStation()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemSocket = CreateDefaultSubobject<USocketComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(RootComponent);
}

void ADeliveryStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	bool bValidDelivery = true;

	if (bValidDelivery)
	{
		UE_LOG(LogTemp, Log, TEXT("DeliveryStation: Delivery completed successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DeliveryStation: Delivery failed validation"));
	}
}
