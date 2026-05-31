#include "PotionPanicOptionCycle.h"

#include "Components/Button.h"
#include "Blueprint/WidgetNavigation.h"

void UPotionPanicOptionCycle::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
}

void UPotionPanicOptionCycle::NativeConstruct()
{
	Super::NativeConstruct();

	FCustomWidgetNavigationDelegate NavDelegate;
	NavDelegate.BindDynamic(this, &UPotionPanicOptionCycle::HandleCycleNavigation);
	SetNavigationRuleCustom(EUINavigation::Left, NavDelegate);
	SetNavigationRuleCustom(EUINavigation::Right, NavDelegate);
}

UWidget* UPotionPanicOptionCycle::HandleCycleNavigation(EUINavigation Direction)
{
	if (Direction == EUINavigation::Left && BTN_Previous)
	{
		BTN_Previous->OnClicked.Broadcast();
	}
	else if (Direction == EUINavigation::Right && BTN_Next)
	{
		BTN_Next->OnClicked.Broadcast();
	}

	return this;
}

void UPotionPanicOptionCycle::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	SetHighlighted(true);
}

void UPotionPanicOptionCycle::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	SetHighlighted(false);
}

void UPotionPanicOptionCycle::SetHighlighted(bool bHighlighted)
{
	SetRenderScale(bHighlighted ? FVector2D(HighlightScale, HighlightScale) : FVector2D(1.0f, 1.0f));
}
