#include "TrashStation.h"
#include "ItemAsset.h"
#include "Engine/AssetManager.h"

bool ATrashStation::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	// Check if item is loaded to verify destructibility
	// If not loaded, we assume it's valid to be trashed (or we could force load, but let's avoid sync loads)
	UItemAsset* Item = Cast<UItemAsset>(UAssetManager::Get().GetPrimaryAssetObject(ItemId));
	
	// Logic: If Item IS NOT Destructible (i.e. is Indestructible), we CANNOT trash it. -- Fix logic
	if (Item && !Item->bIsDestructible)
	{
		return false;
	}
	return Super::CanPlaceItem(ItemId);
}
