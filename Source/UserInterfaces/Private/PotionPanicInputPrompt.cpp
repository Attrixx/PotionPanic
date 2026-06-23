#include "PotionPanicInputPrompt.h"

#include "CommonActionWidget.h"
#include "CommonTextBlock.h"
#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"

void UPotionPanicInputPrompt::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (InputAction && GamepadIcon)
	{
		GamepadIcon->SetEnhancedInputAction(InputAction);
	}
}

void UPotionPanicInputPrompt::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputMethodChangedHandle = InputSubsystem->OnInputMethodChangedNative.AddUObject(
			this, &UPotionPanicInputPrompt::HandleInputMethodChanged);
	}

	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningLocalPlayer()))
	{
		EnhancedInput->ControlMappingsRebuiltDelegate.AddDynamic(
			this, &UPotionPanicInputPrompt::HandleControlMappingsRebuilt);
	}

	RefreshPrompt();
}

void UPotionPanicInputPrompt::HandleControlMappingsRebuilt()
{
	RefreshPrompt();
}

void UPotionPanicInputPrompt::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
	}
	InputMethodChangedHandle.Reset();

	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningLocalPlayer()))
	{
		EnhancedInput->ControlMappingsRebuiltDelegate.RemoveDynamic(
			this, &UPotionPanicInputPrompt::HandleControlMappingsRebuilt);
	}

	Super::NativeDestruct();
}

void UPotionPanicInputPrompt::SetInputAction(UInputAction* InInputAction)
{
	InputAction = InInputAction;

	if (GamepadIcon)
	{
		GamepadIcon->SetEnhancedInputAction(InputAction);
	}

	RefreshPrompt();
}

void UPotionPanicInputPrompt::HandleInputMethodChanged(ECommonInputType )
{
	RefreshPrompt();
}

void UPotionPanicInputPrompt::RefreshPrompt()
{
	if (!GamepadIcon || !KeyboardText)
	{
		return;
	}

	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const bool bUsingGamepad = InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;

	if (bUsingGamepad)
	{
		GamepadIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		KeyboardText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		const FKey KeyboardKey = ResolveKeyboardKey();
		KeyboardText->SetText(KeyboardKey.IsValid() ? KeyboardKey.GetDisplayName() : FText::GetEmpty());
		KeyboardText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		GamepadIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FKey UPotionPanicInputPrompt::ResolveKeyboardKey() const
{
	if (!InputAction)
	{
		return EKeys::Invalid;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const UEnhancedInputLocalPlayerSubsystem* EnhancedInput =
		LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!EnhancedInput)
	{
		return EKeys::Invalid;
	}

	for (const FKey& Key : EnhancedInput->QueryKeysMappedToAction(InputAction))
	{
		if (!Key.IsGamepadKey())
		{
			return Key;
		}
	}

	return EKeys::Invalid;
}
