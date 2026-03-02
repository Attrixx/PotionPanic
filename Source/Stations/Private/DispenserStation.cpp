#include "DispenserStation.h"
#include "CarriableComponent.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemSpawnLibrary.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(MS_DispenserStation, Log, All);

void ADispenserStation::Interact(APlayerController& InInstigator)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* Pawn = InInstigator.GetPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	UHolderComponent* PlayerHolder = Pawn->FindComponentByClass<UHolderComponent>();
	if (PlayerHolder == nullptr)
	{
		return;
	}

	if (PlayerHolder->GetCarriable() != nullptr)
	{
		return;
	}

	if (!ItemToDispense.IsValid())
	{
		UE_LOGFMT(MS_DispenserStation, Warning, "ItemToDispense is not configured for station '{0}'.", GetName());
		return;
	}

	const FTransform SpawnTransform = ItemHolder ? ItemHolder->GetComponentTransform() : GetActorTransform();
	AItemActor* SpawnedItem = UItemSpawnLibrary::SpawnItemFromPrimaryAssetId(this, ItemToDispense, SpawnTransform, ItemActorClass);
	if (SpawnedItem == nullptr)
	{
		UE_LOGFMT(MS_DispenserStation, Warning, "Failed to spawn item '{0}' on station '{1}'.", ItemToDispense.ToString(), GetName());
		return;
	}

	UCarriableComponent* SpawnedCarriable = SpawnedItem->FindComponentByClass<UCarriableComponent>();
	if (SpawnedCarriable == nullptr)
	{
		UE_LOGFMT(MS_DispenserStation, Warning, "Spawned item '{0}' has no UCarriableComponent.", SpawnedItem->GetName());
		SpawnedItem->DestroyItem(true);
		return;
	}

	PlayerHolder->Replace(SpawnedCarriable);
}
