#include "PotionPanicActivatableWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"

namespace
{

	UWidget* FindFirstFocusableWidget(UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (const TSharedPtr<SWidget> Slate = Widget->GetCachedWidget(); Slate.IsValid())
		{
			if (!Slate->GetVisibility().IsVisible())
			{
				return nullptr;
			}
			if (Slate->SupportsKeyboardFocus())
			{
				return Widget;
			}
		}

		if (const UUserWidget* AsUserWidget = Cast<UUserWidget>(Widget))
		{
			return AsUserWidget->WidgetTree ? FindFirstFocusableWidget(AsUserWidget->WidgetTree->RootWidget) : nullptr;
		}

		if (const UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
			{
				if (UWidget* Found = FindFirstFocusableWidget(Panel->GetChildAt(i)))
				{
					return Found;
				}
			}
		}

		return nullptr;
	}

	UWidget* FindWidgetByName(UWidget* Widget, const FName& WidgetName)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (Widget->GetFName() == WidgetName)
		{
			return Widget;
		}

		if (const UUserWidget* AsUserWidget = Cast<UUserWidget>(Widget))
		{
			return AsUserWidget->WidgetTree ? FindWidgetByName(AsUserWidget->WidgetTree->RootWidget, WidgetName) : nullptr;
		}

		if (const UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
			{
				if (UWidget* Found = FindWidgetByName(Panel->GetChildAt(i), WidgetName))
				{
					return Found;
				}
			}
		}

		return nullptr;
	}
}

UPotionPanicActivatableWidget::UPotionPanicActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsBackHandler = true;
	bAutoActivate = true;
}

void UPotionPanicActivatableWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([](UWidget* Widget)
		{
			if (UCommonAnimatedSwitcher* Switcher = Cast<UCommonAnimatedSwitcher>(Widget))
			{
				Switcher->SetDisableTransitionAnimation(true);
			}
		});
	}
}

TOptional<FUIInputConfig> UPotionPanicActivatableWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(InputMode, MouseCaptureMode);
}

UWidget* UPotionPanicActivatableWidget::NativeGetDesiredFocusTarget() const
{
	// Only hand CommonUI an initial focus target when navigating with a gamepad. With mouse/keyboard
	// we don't want the first element auto-focused (and highlighted) when the screen activates.
	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		if (InputSubsystem->GetCurrentInputType() != ECommonInputType::Gamepad)
		{
			return nullptr;
		}
	}

	if (!DesiredFocusWidgetName.IsNone())
	{
		if (UWidget* ByName = FindWidgetByName(GetRootWidget(), DesiredFocusWidgetName))
		{
			return ByName;
		}
	}

	if (UWidget* FirstFocusable = FindFirstFocusableWidget(GetRootWidget()))
	{
		return FirstFocusable;
	}

	return Super::NativeGetDesiredFocusTarget();
}

FReply UPotionPanicActivatableWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if ((Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape) && bIsBackHandler)
	{
		if (NativeOnBackAction())
		{
			return FReply::Handled();
		}
		BP_OnBackAction();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
