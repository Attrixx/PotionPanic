#include "StationDataAsset.h"

void UStationDataAsset::PostLoad()
{
	Super::PostLoad();

	if (Instructions.Num() == 0 && Recipes_DEPRECATED.Num() > 0)
	{
		Instructions = Recipes_DEPRECATED;
	}
}
