#include "InteractionBase.h"

#include "InteractionSetting.h"
#include "UObject/GarbageCollectionSchema.h"

DEFINE_LOG_CATEGORY_STATIC(MS_InteractionBase, Log, All);

UInteractionBase* UInteractionBase::CreateInteraction(UObject* Outer, UInteractionSetting* Settings)
{
	if (!Settings)
	{
		UE_LOGFMT(MS_InteractionBase, Error, "Cannot create an interaction from null Settings");
		return nullptr;
	}
	
	if (!Settings->InteractionClass)
	{
		UE_LOGFMT(MS_InteractionBase, Error, "Settings Interaction class cannot be nullptr to create one");
		return nullptr;
	}
	
	UInteractionBase* NewInteraction = NewObject<UInteractionBase>(Outer, Settings->InteractionClass);
	return NewInteraction;
}
