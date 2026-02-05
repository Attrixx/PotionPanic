#include "TrashStation.h"
#include "ItemAsset.h"

bool ATrashStation::CanPlaceItem(const UItemAsset* Item) const
{
	if (Item && Item->bIsIndestructible)
	{
		return false;
	}
	return Super::CanPlaceItem(Item);
}


