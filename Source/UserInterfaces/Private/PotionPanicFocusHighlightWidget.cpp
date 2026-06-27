#include "PotionPanicFocusHighlightWidget.h"

void UPotionPanicFocusHighlightWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	bFocusHighlight = true;
	RefreshHighlight();
}

void UPotionPanicFocusHighlightWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	bFocusHighlight = false;
	RefreshHighlight();
}

void UPotionPanicFocusHighlightWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bHoverHighlight = true;
	RefreshHighlight();
}

void UPotionPanicFocusHighlightWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bHoverHighlight = false;
	RefreshHighlight();
}

void UPotionPanicFocusHighlightWidget::RefreshHighlight()
{
	const bool bHighlighted = bFocusHighlight || bHoverHighlight;
	if (bHighlighted == bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = bHighlighted;
	UpdateHighlight(bHighlighted);
}
