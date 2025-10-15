#include "DistinguishSystem/WidgetDistinguishComponent.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(MS_WidgetDistinguishComponent, Log, All);

UWidgetDistinguishComponent::UWidgetDistinguishComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWidgetDistinguishComponent::BeginPlay()
{
	Super::BeginPlay();

	// This component should always be provided with a widget reference
	if (!UserWidget)
	{
		UE_LOGFMT(MS_WidgetDistinguishComponent, Error, "WidgetDistinguishComponent on {0} has no UserWidget set.", GetOwner()->GetName());
	}
}

void UWidgetDistinguishComponent::OnActivate()
{
	if (!UserWidget)
		return;
}

void UWidgetDistinguishComponent::OnDeactivate()
{
	if (!UserWidget)
		return;
}