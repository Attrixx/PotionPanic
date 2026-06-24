#include "Components/QTEDisplayComponent.h"

#include "Components/QTEComponent.h"
#include "Widgets/QTEWidgetBase.h"

UQTEDisplayComponent::UQTEDisplayComponent()
{
	SetIsReplicatedByDefault(true);

	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetVisibility(false);
	SetRelativeLocation(FVector(0.f, 0.f, 180.f));
}

void UQTEDisplayComponent::ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass)
{
	Client_ShowQTEActivityStep(InQTEComponent, InWidgetClass);
}

void UQTEDisplayComponent::HideQTEActivityStep()
{
	Client_HideQTEActivityStep();
}

UQTEWidgetBase* UQTEDisplayComponent::GetQTEWidget() const
{
	return Cast<UQTEWidgetBase>(GetUserWidgetObject());
}

void UQTEDisplayComponent::Client_ShowQTEActivityStep_Implementation(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass)
{
	if (InWidgetClass && GetWidgetClass() != InWidgetClass)
	{
		SetWidgetClass(InWidgetClass);
		InitWidget();
	}

	if (UQTEWidgetBase* QTEWidget = GetQTEWidget())
	{
		UQTEComponent* ComponentToBind = InQTEComponent ? InQTEComponent : (GetOwner() ? GetOwner()->FindComponentByClass<UQTEComponent>() : nullptr);
		QTEWidget->BindToQTEComponent(ComponentToBind);
		SetVisibility(true);
	}
}

void UQTEDisplayComponent::Client_HideQTEActivityStep_Implementation()
{
	if (UQTEWidgetBase* QTEWidget = GetQTEWidget())
	{
		QTEWidget->BindToQTEComponent(nullptr);
	}

	SetVisibility(false);
}
