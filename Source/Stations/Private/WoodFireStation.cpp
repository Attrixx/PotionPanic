#include "WoodFireStation.h"
#include "ItemAsset.h"
#include "Cauldron.h"
#include "HolderComponent.h"
#include "CarriableComponent.h"

bool AWoodFireStation::CanPlaceItem(const UItemAsset* Item) const
{
	if (!RequiredItem)
	{
		return Super::CanPlaceItem(Item);
	}

	return Item == RequiredItem;
}

void AWoodFireStation::StartProcessing(const FInstruction& Instruction)
{
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();
	if (StationItem)
	{
		ACauldron* Cauldron = Cast<ACauldron>(StationItem->GetOwner());
		if (Cauldron)
		{
            // TODO (Nath): Implement Recipe Check here.
		}
	}

	Super::StartProcessing(Instruction);
}

void AWoodFireStation::FinishProcessing()
{
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();
	if (StationItem)
	{
		ACauldron* Cauldron = Cast<ACauldron>(StationItem->GetOwner());
		if (Cauldron)
		{
            // TODO (Nath): Implement Recipe Result here.
            
			StationState = EStationState::Completed;
			SetActorTickEnabled(false);
            
            OnFinishProcessingBP(CurrentInstruction);
			return;
		}
	}

	Super::FinishProcessing();
}
