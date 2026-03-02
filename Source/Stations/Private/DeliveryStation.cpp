#include "DeliveryStation.h"
#include "CarriableComponent.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "ItemActor.h"

void ADeliveryStation::Interact(APlayerController& InInstigator)
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

	UCarriableComponent* HeldCarriable = PlayerHolder->GetCarriable();
	if (HeldCarriable == nullptr)
	{
		return;
	}

	const FPrimaryAssetId HeldItemId = HeldCarriable->GetItemId();
	if (!CanPlaceItem(HeldItemId))
	{
		return;
	}

	UCarriableComponent* RemovedCarriable = PlayerHolder->Replace(nullptr);
	if (RemovedCarriable == nullptr)
	{
		return;
	}

	if (AItemActor* ItemActor = Cast<AItemActor>(RemovedCarriable->GetOwner()))
	{
		ItemActor->DestroyItem(true);
	}
	else if (AActor* OwnerActor = RemovedCarriable->GetOwner())
	{
		OwnerActor->Destroy();
	}

	OnItemDelivered.Broadcast(HeldItemId);
}

bool ADeliveryStation::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	if (AcceptedItems.Num() == 0)
	{
		return true;
	}

	return AcceptedItems.Contains(ItemId);
}
