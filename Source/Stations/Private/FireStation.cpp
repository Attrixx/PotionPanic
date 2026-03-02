#include "FireStation.h"
#include "CarriableComponent.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"

AFireStation::AFireStation()
{
	bAcceptContainerItems = true;
}

void AFireStation::Interact(APlayerController& InInstigator)
{
	if (!HasAuthority() || ItemHolder == nullptr)
	{
		return;
	}

	APawn* InstigatorPawn = InInstigator.GetPawn();
	UHolderComponent* PlayerHolder = InstigatorPawn ? InstigatorPawn->FindComponentByClass<UHolderComponent>() : nullptr;
	if (PlayerHolder == nullptr)
	{
		return;
	}

	UCarriableComponent* PlayerCarriable = PlayerHolder->GetCarriable();
	UCarriableComponent* StationCarriable = ItemHolder->GetCarriable();
	const bool bPlayerHasItem = PlayerCarriable != nullptr;
	const bool bStationHasItem = StationCarriable != nullptr;

	if (!bPlayerHasItem && bStationHasItem)
	{
		if (StationState == EStationState::Processing || GetQueuedInstructionCount() > 0)
		{
			OnProcessRequested.Broadcast(&InInstigator, this);
			return;
		}

		UCarriableComponent* MovedCarriable = ItemHolder->Replace(nullptr);
		if (MovedCarriable != nullptr)
		{
			PlayerHolder->Replace(MovedCarriable);
		}
		return;
	}

	if (bPlayerHasItem)
	{
		if (bStationHasItem || !CanPlaceItem(PlayerCarriable->GetItemId()))
		{
			return;
		}

		UCarriableComponent* MovedCarriable = PlayerHolder->Replace(nullptr);
		if (MovedCarriable == nullptr)
		{
			return;
		}

		ItemHolder->Replace(MovedCarriable);
		OnProcessRequested.Broadcast(&InInstigator, this);
		return;
	}
}

bool AFireStation::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	if (!RequiredCauldronItemId.IsValid())
	{
		return false;
	}

	if (ItemId != RequiredCauldronItemId)
	{
		return false;
	}

	return Super::CanPlaceItem(ItemId);
}
