#include "ControlsSettingsWidget.h"
#include "KeybindManager.h"
#include "PotionPanicKeybindSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"

UControlsSettingsWidget::UControlsSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	bAutoActivate = false;
	bIsBackHandler = false;
	SetIsFocusable(false);
}

void UControlsSettingsWidget::InitializeBindings()
{
	KeybindManager = NewObject<UKeybindManager>(this);
	KeybindManager->DisplayNameOverrides = DisplayNameOverrides;

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		KeybindManager->SetPlayerIndex(LocalPlayer->GetControllerId());
	}

	KeybindManager->InitializeFromIMC(InputMappingContext);
	KeybindManager->Load();

	OnBindingsRefreshed(KeybindManager->GetBindingsForDevice(false));
}

void UControlsSettingsWidget::StartRebinding(FName ActionName, int32 MappingIndex)
{
	RebindingActionName   = ActionName;
	RebindingMappingIndex = MappingIndex;
	bIsListeningForInput  = true;
	SetKeyboardFocus();
}

void UControlsSettingsWidget::CancelRebinding()
{
	bIsListeningForInput  = false;
	RebindingActionName   = NAME_None;
	RebindingMappingIndex = 0;
}

void UControlsSettingsWidget::ResetBinding(FName ActionName, int32 MappingIndex)
{
	if (!KeybindManager) return;

	KeybindManager->ResetBinding(ActionName, MappingIndex);
	OnBindingsRefreshed(KeybindManager->GetBindingsForDevice(false));
}

void UControlsSettingsWidget::ResetAllBindings()
{
	if (!KeybindManager) return;

	KeybindManager->ResetAllBindings();
	OnBindingsRefreshed(KeybindManager->GetBindingsForDevice(false));
}

TArray<FKeybindEntry> UControlsSettingsWidget::GetBindings(bool bGamepad) const
{
	if (!KeybindManager) return TArray<FKeybindEntry>();
	return KeybindManager->GetBindingsForDevice(bGamepad);
}

void UControlsSettingsWidget::ProcessRebind(const FKey& NewKey)
{
	if (!KeybindManager) return;

	const FName ActionName   = RebindingActionName;
	const int32 MappingIndex = RebindingMappingIndex;
	const bool bIsGamepad    = NewKey.IsGamepadKey();

	bIsListeningForInput  = false;
	RebindingActionName   = NAME_None;
	RebindingMappingIndex = 0;

	if (KeybindManager->HasDuplicateKey(NewKey, bIsGamepad, ActionName, MappingIndex))
	{
		FKeybindEntry Duplicate = KeybindManager->GetDuplicateEntry(NewKey, bIsGamepad, ActionName, MappingIndex);
		OnDuplicateKeyDetected(NewKey, Duplicate.DisplayName);
		return;
	}

	KeybindManager->RebindKey(ActionName, MappingIndex, NewKey, bIsGamepad);
	OnKeyRebound(ActionName, MappingIndex, NewKey);
}

FReply UControlsSettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bIsListeningForInput && RebindingActionName != NAME_None)
	{
		const FKey PressedKey = InKeyEvent.GetKey();
		if (!PressedKey.IsModifierKey())
		{
			ProcessRebind(PressedKey);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UControlsSettingsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsListeningForInput && RebindingActionName != NAME_None)
	{
		ProcessRebind(InMouseEvent.GetEffectingButton());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UControlsSettingsWidget::ApplyIfDirty()
{
	if (!KeybindManager || !KeybindManager->IsDirty()) return;
	Apply();
}

void UControlsSettingsWidget::Apply()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (!Subsystem || !InputMappingContext || !KeybindManager) return;

	UInputMappingContext* TargetContext = InputMappingContext;
	if (UPotionPanicKeybindSubsystem* KeybindSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UPotionPanicKeybindSubsystem>() : nullptr)
	{
		TargetContext = KeybindSubsystem->GetRuntimeContext(InputMappingContext);
	}

	KeybindManager->ApplyToIMC(TargetContext, Subsystem);
	KeybindManager->Save();
}
