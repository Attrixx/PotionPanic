#include "DeliveryStation.h"
#include "ItemAsset.h"
#include "ItemActor.h"
#include "HolderComponent.h"
#include "CarriableComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(MS_DeliveryStation, Log, All);

ADeliveryStation::ADeliveryStation()
{
}

bool ADeliveryStation::CanPlaceItem(const UItemAsset* Item) const
{
	if (Item && Item->bIsIndestructible)
	{
		return false;
	}
	return true; 
}

void ADeliveryStation::Interact(APlayerController& InInstigator)
{
	if (!InInstigator.GetPawn()) return;
	
	UHolderComponent* PlayerHolder = InInstigator.GetPawn()->FindComponentByClass<UHolderComponent>();
	if (!PlayerHolder) return;

	UCarriableComponent* PlayerItem = PlayerHolder->GetCarriable();
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();

	if (PlayerItem && !StationItem)
	{
        AItemActor* ItemActor = Cast<AItemActor>(PlayerItem->GetOwner());
		if (ItemActor && !CanPlaceItem(ItemActor->GetItemAsset()))
		{
			return;
		}

		UCarriableComponent* Old = PlayerHolder->Replace(nullptr);
		ItemHolder->Replace(Old);
		
		FInstruction DummyInstr;
		DummyInstr.ProcessingDuration = 0.5f; 
		StartProcessing(DummyInstr);
	}
}

void ADeliveryStation::StartProcessing(const FInstruction& Instruction)
{	
	Super::StartProcessing(Instruction);
}

void ADeliveryStation::FinishProcessing()
{
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();
	if (StationItem)
	{
		AItemActor* ItemActor = Cast<AItemActor>(StationItem->GetOwner());
		if (ItemActor && ItemActor->GetItemAsset())
		{
			// TODO (Nath): Verify if this Item matches the current Order/Objective
			UE_LOGFMT(MS_DeliveryStation, Log, "Delivered Item: {0}", ItemActor->GetItemAsset()->GetName());
			
			// Score Logic here (ScoreSubsystem->AddScore(...))
		}
		
		ItemActor->Destroy();
		ItemHolder->Replace(nullptr);
	}
	
	StationState = EStationState::Completed;
	OnStationStateChangedBP(StationState);
	SetActorTickEnabled(false);
}
