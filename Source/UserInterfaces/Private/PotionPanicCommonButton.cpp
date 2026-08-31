#include "PotionPanicCommonButton.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateBrush.h"

TSharedRef<SWidget> UPotionPanicCommonButton::RebuildWidget()
{
	if (!IsDesignTime())
	{
		static bool bFocusRectangleCleared = false;
		if (!bFocusRectangleCleared)
		{
			bFocusRectangleCleared = true;

			FSlateStyleSet& CoreStyle = const_cast<FSlateStyleSet&>(static_cast<const FSlateStyleSet&>(FCoreStyle::Get()));
			FSlateBrush* EmptyBrush = new FSlateBrush();
			EmptyBrush->DrawAs = ESlateBrushDrawType::NoDrawType;
			CoreStyle.Set("FocusRectangle", EmptyBrush);
		}
	}

	return Super::RebuildWidget();
}

void UPotionPanicCommonButton::NativeOnHovered()
{
	Super::NativeOnHovered();
	bIsHoveredHighlight = true;
	RefreshHighlight();
}

void UPotionPanicCommonButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	bIsHoveredHighlight = false;
	RefreshHighlight();
}

void UPotionPanicCommonButton::HandleFocusReceived()
{
	Super::HandleFocusReceived();
	bIsFocusedHighlight = true;
	RefreshHighlight();
}

void UPotionPanicCommonButton::HandleFocusLost()
{
	Super::HandleFocusLost();
	bIsFocusedHighlight = false;
	RefreshHighlight();
}

void UPotionPanicCommonButton::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);
	RefreshHighlight();
}

void UPotionPanicCommonButton::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);
	RefreshHighlight();
}

void UPotionPanicCommonButton::RefreshHighlight()
{
	// Selection only pins the highlight for toggle buttons (e.g. the active settings tab).
	// A regular action button must not stay highlighted after a click once the mouse leaves.
	const bool bSelectedHighlight = bToggleable && GetSelected();
	const bool bHighlighted = bIsHoveredHighlight || bIsFocusedHighlight || bSelectedHighlight;

	// Only drive the visual on an actual state change, otherwise hovering an already-highlighted
	// (e.g. selected) button would replay the animation from the start.
	if (bHighlighted == bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = bHighlighted;
	UpdateHighlight(bHighlighted);
}
