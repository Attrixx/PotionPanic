#include "TrashStation.h"
#include "CarriableComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "IngredientData.h"
#include "ItemActor.h"
#include "ItemAsset.h"

void ATrashStation::Interact(APlayerController& InInstigator)
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

	if (!CanPlaceItem(HeldCarriable->GetItemId()))
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
}

bool ATrashStation::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	UItemAsset* Item = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemId);
	if (Item == nullptr && ItemId.IsValid())
	{
		const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
		Item = Cast<UItemAsset>(AssetPath.TryLoad());
	}

	return Cast<UIngredientData>(Item) != nullptr;
}
