#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PotionPanicFocusHighlightWidget.generated.h"

/**
 * UserWidget base that raises a single UpdateHighlight(bool) Blueprint event when the widget
 * (or one of its descendants) gains/loses focus, or is hovered. The event only fires on an actual
 * state change so a Blueprint highlight animation is never replayed redundantly.
 */
UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicFocusHighlightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHighlight(bool bHighlighted);

private:

	void RefreshHighlight();

	bool bFocusHighlight = false;
	bool bHoverHighlight = false;
	bool bIsHighlighted = false;
};
