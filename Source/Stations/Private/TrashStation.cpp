#include "TrashStation.h"
#include "ItemActor.h"
#include "SocketComponent.h"

ATrashStation::ATrashStation()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemSocket = CreateDefaultSubobject<USocketComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(RootComponent);
}

void ATrashStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (!CurrentItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrashStation: No item to process"));
		return;
	}

	// Check if the item has attached items (is a container)
	TArray<AActor*> AttachedActors;
	CurrentItem->GetAttachedActors(AttachedActors);

	// Filter to only get AItemActor children
	TArray<AItemActor*> AttachedItems;
	for (AActor* Actor : AttachedActors)
	{
		if (AItemActor* Item = Cast<AItemActor>(Actor))
		{
			AttachedItems.Add(Item);
		}
	}

	// If item has contents (is a container like a cauldron), destroy only the contents
	if (AttachedItems.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("TrashStation: Emptying container '%s' - destroying %d items"), 
			*CurrentItem->GetName(), AttachedItems.Num());

		for (AItemActor* Item : AttachedItems)
		{
			if (Item)
			{
				UE_LOG(LogTemp, Log, TEXT("  - Destroying content: %s"), *Item->GetName());
				Item->Destroy();
			}
		}

		// Clear CurrentItem reference but don't destroy the container
		CurrentItem = nullptr;
	}
	else
	{
		// Simple item with no contents - destroy it entirely
		UE_LOG(LogTemp, Log, TEXT("TrashStation: Destroying simple item '%s'"), *CurrentItem->GetName());
		CurrentItem->Destroy();
		CurrentItem = nullptr;
	}
}
