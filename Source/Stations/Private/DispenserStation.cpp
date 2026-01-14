#include "DispenserStation.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "Engine/AssetManager.h"

ADispenserStation::ADispenserStation()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADispenserStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (!bUnlimitedSupply && CurrentStock <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DispenserStation: Out of stock!"));
		return;
	}

	AItemActor* SpawnedItem = SpawnItemFromAsset(Instruction.OutputItem);

	if (SpawnedItem && !bUnlimitedSupply)
	{
		CurrentStock--;
		UE_LOG(LogTemp, Log, TEXT("DispenserStation: Dispensed item. Stock remaining: %d"), CurrentStock);
	}
}

AItemActor* ADispenserStation::SpawnItemFromAsset(const FPrimaryAssetId& AssetId)
{
	if (!ItemActorClass || !AssetId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DispenserStation: Invalid ItemActorClass or AssetId"));
		return nullptr;
	}
	
	FVector SpawnLocation = GetActorLocation() + SpawnOffset;
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (NewItem)
	{
		UE_LOG(LogTemp, Log, TEXT("DispenserStation: Spawned item"));
	}

	return NewItem;
}
