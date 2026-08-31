#include "PotionPanicInputIconLibrary.h"

#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Styling/SlateBrush.h"

bool UPotionPanicInputIconLibrary::GetGamepadIconForKey(const ULocalPlayer* LocalPlayer, FKey Key, FSlateBrush& OutBrush)
{
	if (!Key.IsValid() || !Key.IsGamepadKey())
	{
		return false;
	}

	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
	if (!InputSubsystem)
	{
		return false;
	}

	return UCommonInputPlatformSettings::Get()->TryGetInputBrush(
		OutBrush, Key, ECommonInputType::Gamepad, InputSubsystem->GetCurrentGamepadName());
}

void UPotionPanicInputIconLibrary::ApplyKeyDisplay(UImage* IconImage, UTextBlock* KeyText, const ULocalPlayer* LocalPlayer, FKey Key)
{
	FSlateBrush Brush;
	const bool bGamepadIcon = GetGamepadIconForKey(LocalPlayer, Key, Brush);

	if (IconImage)
	{
		if (bGamepadIcon)
		{
			IconImage->SetBrush(Brush);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (KeyText)
	{
		if (bGamepadIcon)
		{
			KeyText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			KeyText->SetText(Key.GetDisplayName());
			KeyText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}
