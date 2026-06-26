#include "StationAsset.h"
#include "StationVisualActor.h"
#include <Misc/DataValidation.h>

#if WITH_EDITOR
EDataValidationResult UStationAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!IsValid(VisualActorClass))
	{
		Context.AddError(INVTEXT("Visual Actor Class is null."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
