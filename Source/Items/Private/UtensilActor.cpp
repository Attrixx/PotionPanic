#include "UtensilActor.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(MS_UtensilActor, Log, All);

AUtensilActor::AUtensilActor()
{
	// Default settings for utensils
	SetCanBeDamaged(false);
}

void AUtensilActor::DestroyItem(bool bPlayFeedback)
{
	UE_LOGFMT(MS_UtensilActor, Verbose, "DestroyItem ignored for utensil '{0}'.", GetName());
}
