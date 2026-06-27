#include "ControlsSettingsWidget.h"
#include "KeybindManager.h"
#include "PotionPanicKeybindSubsystem.h"
#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/ScopeExit.h"

class FRebindKeyPreprocessor : public IInputProcessor
{
public:
	DECLARE_DELEGATE_OneParam(FOnKeyCaptured, FKey);
	FOnKeyCaptured OnKeyCaptured;

	virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication&, const FKeyEvent& InKeyEvent) override
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key.IsModifierKey())
		{
			return false;
		}
		OnKeyCaptured.ExecuteIfBound(Key);
		return true;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication&, const FPointerEvent& InMouseEvent) override
	{
		OnKeyCaptured.ExecuteIfBound(InMouseEvent.GetEffectingButton());
		return true;
	}
};

namespace
{
	bool GetRowAction(const UUserWidget* Row, FName& OutAction, int32& OutIndex)
	{
		if (!Row)
		{
			return false;
		}
		const FNameProperty* NameProp = FindFProperty<FNameProperty>(Row->GetClass(), TEXT("ActionName"));
		if (!NameProp)
		{
			return false;
		}
		OutAction = NameProp->GetPropertyValue_InContainer(Row);
		const FIntProperty* IndexProp = FindFProperty<FIntProperty>(Row->GetClass(), TEXT("MappingIndex"));
		OutIndex = IndexProp ? IndexProp->GetPropertyValue_InContainer(Row) : 0;
		return true;
	}

	UWidget* FindFirstFocusable(UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}
		if (const TSharedPtr<SWidget> Slate = Widget->GetCachedWidget(); Slate.IsValid())
		{
			if (Slate->GetVisibility().IsVisible() && Slate->SupportsKeyboardFocus())
			{
				return Widget;
			}
		}
		if (const UUserWidget* AsUserWidget = Cast<UUserWidget>(Widget))
		{
			UWidget* Match = nullptr;
			if (AsUserWidget->WidgetTree)
			{
				AsUserWidget->WidgetTree->ForEachWidget([&Match, AsUserWidget](UWidget* Child)
				{
					if (!Match && Child != AsUserWidget)
					{
						Match = FindFirstFocusable(Child);
					}
				});
			}
			if (Match)
			{
				return Match;
			}
		}
		if (const UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
			{
				if (UWidget* Found = FindFirstFocusable(Panel->GetChildAt(i)))
				{
					return Found;
				}
			}
		}
		return nullptr;
	}

	void CollectRows(UWidget* Widget, TArray<UUserWidget*>& Out)
	{
		if (!Widget)
		{
			return;
		}
		if (UUserWidget* AsUserWidget = Cast<UUserWidget>(Widget))
		{
			FName A; int32 I;
			if (GetRowAction(AsUserWidget, A, I))
			{
				Out.Add(AsUserWidget);
				return;
			}
		}
		if (const UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
			{
				CollectRows(Panel->GetChildAt(i), Out);
			}
		}
	}
}

UControlsSettingsWidget::UControlsSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = false;
	bIsBackHandler = false;
	SetIsFocusable(true);
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

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		if (!InputMethodChangedHandle.IsValid())
		{
			InputMethodChangedHandle = InputSubsystem->OnInputMethodChangedNative.AddUObject(
				this, &UControlsSettingsWidget::HandleInputMethodChanged);
		}
	}

	RefreshList();
}

void UControlsSettingsWidget::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		InputSubsystem->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
	}
	InputMethodChangedHandle.Reset();
	UnregisterRebindPreprocessor();

	Super::NativeDestruct();
}

void UControlsSettingsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	FocusListDeferred();
}

void UControlsSettingsWidget::FocusListDeferred()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::FocusListNow);
	}
}

void UControlsSettingsWidget::FocusListNow()
{
	ON_SCOPE_EXIT
	{
		PendingFocusActionName   = NAME_None;
		PendingFocusMappingIndex = 0;
	};

	if (!IsUsingGamepad())
	{
		return;
	}

	UWidget* Target = nullptr;
	if (!PendingFocusActionName.IsNone())
	{
		Target = FindRowFocusWidget(PendingFocusActionName, PendingFocusMappingIndex);
	}
	if (!Target)
	{
		Target = FindFirstRowFocusWidget();
	}

	if (Target)
	{
		Target->SetFocus();
	}
}

UWidget* UControlsSettingsWidget::FindRowFocusWidget(FName ActionName, int32 MappingIndex) const
{
	TArray<UUserWidget*> Rows;
	CollectRows(GetRootWidget(), Rows);
	for (UUserWidget* Row : Rows)
	{
		FName A; int32 I;
		if (GetRowAction(Row, A, I) && A == ActionName && I == MappingIndex)
		{
			return FindFirstFocusable(Row);
		}
	}
	return nullptr;
}

UWidget* UControlsSettingsWidget::FindFirstRowFocusWidget() const
{
	TArray<UUserWidget*> Rows;
	CollectRows(GetRootWidget(), Rows);
	return Rows.Num() > 0 ? FindFirstFocusable(Rows[0]) : nullptr;
}

void UControlsSettingsWidget::RefreshList()
{
	if (!KeybindManager) return;
	OnBindingsRefreshed(KeybindManager->GetBindingsForDevice(IsUsingGamepad()));
}

bool UControlsSettingsWidget::IsUsingGamepad() const
{
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}

void UControlsSettingsWidget::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	RefreshList();

	if (NewInputType == ECommonInputType::Gamepad)
	{
		FocusListDeferred();
	}
}

void UControlsSettingsWidget::StartRebinding(FName ActionName, int32 MappingIndex)
{
	RebindingActionName   = ActionName;
	RebindingMappingIndex = MappingIndex;
	bIsListeningForInput  = true;
	SetKeyboardFocus();
	RegisterRebindPreprocessor();
}

void UControlsSettingsWidget::CancelRebinding()
{
	UnregisterRebindPreprocessor();
	bIsListeningForInput  = false;
	RebindingActionName   = NAME_None;
	RebindingMappingIndex = 0;
}

void UControlsSettingsWidget::RegisterRebindPreprocessor()
{
	if (RebindPreprocessor.IsValid() || !FSlateApplication::IsInitialized())
	{
		return;
	}
	RebindPreprocessor = MakeShared<FRebindKeyPreprocessor>();
	RebindPreprocessor->OnKeyCaptured.BindUObject(this, &UControlsSettingsWidget::HandleRebindKeyCaptured);
	FSlateApplication::Get().RegisterInputPreProcessor(RebindPreprocessor, 0);
}

void UControlsSettingsWidget::UnregisterRebindPreprocessor()
{
	if (RebindPreprocessor.IsValid())
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(RebindPreprocessor);
		}
		RebindPreprocessor.Reset();
	}
}

void UControlsSettingsWidget::HandleRebindKeyCaptured(FKey CapturedKey)
{
	UnregisterRebindPreprocessor();
	if (bIsListeningForInput && RebindingActionName != NAME_None)
	{
		ProcessRebind(CapturedKey);
	}
}

void UControlsSettingsWidget::ResetBinding(FName ActionName, int32 MappingIndex)
{
	if (!KeybindManager) return;

	KeybindManager->ResetBinding(ActionName, MappingIndex);
	RefreshList();
}

void UControlsSettingsWidget::ResetAllBindings()
{
	if (!KeybindManager) return;

	KeybindManager->ResetAllBindings();
	RefreshList();
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

	UnregisterRebindPreprocessor();
	bIsListeningForInput  = false;
	RebindingActionName   = NAME_None;
	RebindingMappingIndex = 0;

	PendingFocusActionName   = ActionName;
	PendingFocusMappingIndex = MappingIndex;

	if (KeybindManager->HasDuplicateKey(NewKey, bIsGamepad, ActionName, MappingIndex))
	{
		const FKeybindEntry Duplicate = KeybindManager->GetDuplicateEntry(NewKey, bIsGamepad, ActionName, MappingIndex);

		FKey OldKey;
		for (const FKeybindEntry& E : KeybindManager->GetAllBindings())
		{
			if (E.InputActionName == ActionName && E.MappingIndex == MappingIndex)
			{
				OldKey = bIsGamepad ? E.GamepadKey : E.KeyboardKey;
				break;
			}
		}

		KeybindManager->RebindKey(Duplicate.InputActionName, Duplicate.MappingIndex, OldKey, bIsGamepad);
		KeybindManager->RebindKey(ActionName, MappingIndex, NewKey, bIsGamepad);
		OnKeyRebound(ActionName, MappingIndex, NewKey);
		FocusListDeferred();
		return;
	}

	KeybindManager->RebindKey(ActionName, MappingIndex, NewKey, bIsGamepad);
	OnKeyRebound(ActionName, MappingIndex, NewKey);
	FocusListDeferred();
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
	if (UPotionPanicKeybindSubsystem* KeybindSubsystem = LocalPlayer->GetSubsystem<UPotionPanicKeybindSubsystem>())
	{
		TargetContext = KeybindSubsystem->GetRuntimeContext(InputMappingContext);
	}

	KeybindManager->ApplyToIMC(TargetContext, Subsystem);
	KeybindManager->Save();
}
