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
	UpdateHighlight();
}

void UPotionPanicCommonButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	UpdateHighlight();
}

void UPotionPanicCommonButton::HandleFocusReceived()
{
	Super::HandleFocusReceived();
	bIsFocusedHighlight = true;
	UpdateHighlight();
}

void UPotionPanicCommonButton::HandleFocusLost()
{
	Super::HandleFocusLost();
	bIsFocusedHighlight = false;
	UpdateHighlight();
}

void UPotionPanicCommonButton::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);
	UpdateHighlight();
}

void UPotionPanicCommonButton::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);
	UpdateHighlight();
}

void UPotionPanicCommonButton::UpdateHighlight()
{
	const bool bHighlight = IsHovered() || bIsFocusedHighlight || GetSelected();
	SetRenderScale(bHighlight ? FVector2D(HighlightScale, HighlightScale) : FVector2D(1.0f, 1.0f));
}
