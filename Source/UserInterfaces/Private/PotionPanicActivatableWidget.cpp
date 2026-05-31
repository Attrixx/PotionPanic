#include "PotionPanicActivatableWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "CommonAnimatedSwitcher.h"

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
			UWidget* Match = nullptr;
			if (AsUserWidget->WidgetTree)
			{
				AsUserWidget->WidgetTree->ForEachWidget([&Match, AsUserWidget](UWidget* Child)
				{
					if (!Match && Child != AsUserWidget)
					{
						if (UWidget* Found = FindFirstFocusableWidget(Child))
						{
							Match = Found;
						}
					}
				});
			}
			return Match;
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

	UWidget* FindWidgetByNameRecursive(const UUserWidget* Root, const FName& WidgetName)
	{
		if (!Root || !Root->WidgetTree)
		{
			return nullptr;
		}

		UWidget* Match = nullptr;
		Root->WidgetTree->ForEachWidget([&Match, &WidgetName](UWidget* Widget)
		{
			if (Match || !Widget)
			{
				return;
			}
			if (Widget->GetFName() == WidgetName)
			{
				Match = Widget;
				return;
			}
			if (const UUserWidget* Nested = Cast<UUserWidget>(Widget))
			{
				if (UWidget* NestedMatch = FindWidgetByNameRecursive(Nested, WidgetName))
				{
					Match = NestedMatch;
				}
			}
		});
		return Match;
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
	if (!DesiredFocusWidgetName.IsNone())
	{
		if (UWidget* ByName = FindWidgetByNameRecursive(this, DesiredFocusWidgetName))
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
