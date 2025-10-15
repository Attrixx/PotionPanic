#include "DistinguishSystem/DistinguishComponent.h"

void UDistinguishComponent::SetActivate(bool bActivate)
{
	if (bIsActivated == bActivate)
		return;

	bIsActivated = bActivate;
	if (bIsActivated)
	{
		OnActivate();
	}
	else
	{
		OnDeactivate();
	}
}
