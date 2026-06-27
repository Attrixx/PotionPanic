#include "SettingsMenuWidget.h"
#include "VideoSettingsWidget.h"
#include "AudioSettingsWidget.h"
#include "ControlsSettingsWidget.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/CommonUIInputTypes.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"

void USettingsMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Tabs.Reset();
	if (WBP_Video)    Tabs.Add(WBP_Video);
	if (WBP_Audio)    Tabs.Add(WBP_Audio);
	if (WBP_Controls) Tabs.Add(WBP_Controls);

	Contents.Reset();
	Contents.Add(WBP_VideoSettings);
	Contents.Add(WBP_AudioSettings);
	Contents.Add(WBP_ControlsSettings);

	for (UCommonButtonBase* Tab : Tabs)
	{
		if (Tab)
		{
			Tab->SetIsSelectable(true);
			Tab->SetIsToggleable(true);
			Tab->SetIsFocusable(false);
		}
	}
}

void USettingsMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (MenuInputContext && !EnhancedInput->HasMappingContext(MenuInputContext))
			{
				EnhancedInput->AddMappingContext(MenuInputContext, 0);
			}
		}
	}

	if (TabNextAction)
	{
		FBindUIActionArgs Args(TabNextAction, false, FSimpleDelegate::CreateUObject(this, &USettingsMenuWidget::HandleTabNext));
		TabActionBindings.Add(RegisterUIActionBinding(Args));
	}
	if (TabPrevAction)
	{
		FBindUIActionArgs Args(TabPrevAction, false, FSimpleDelegate::CreateUObject(this, &USettingsMenuWidget::HandleTabPrev));
		TabActionBindings.Add(RegisterUIActionBinding(Args));
	}

	SelectTab(0);
}

void USettingsMenuWidget::NativeOnDeactivated()
{
	for (FUIActionBindingHandle& Handle : TabActionBindings)
	{
		if (Handle.IsValid())
		{
			Handle.Unregister();
		}
	}
	TabActionBindings.Reset();

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (MenuInputContext && EnhancedInput->HasMappingContext(MenuInputContext))
			{
				EnhancedInput->RemoveMappingContext(MenuInputContext);
			}
		}
	}

	Super::NativeOnDeactivated();
}

void USettingsMenuWidget::HandleTabNext()
{
	SelectTab(CurrentTabIndex + 1);
}

void USettingsMenuWidget::HandleTabPrev()
{
	SelectTab(CurrentTabIndex - 1);
}

void USettingsMenuWidget::ApplyAndClose()
{
	if (WBP_VideoSettings)    WBP_VideoSettings->ApplyIfDirty();
	if (WBP_AudioSettings)    WBP_AudioSettings->ApplyIfDirty();
	if (WBP_ControlsSettings) WBP_ControlsSettings->ApplyIfDirty();

	OnBackClicked.Broadcast();
}

void USettingsMenuWidget::SelectTab(int32 Index)
{
	const int32 Count = Tabs.Num() > 0 ? Tabs.Num() : Contents.Num();
	if (Count <= 0)
	{
		return;
	}

	const int32 NewIndex = ((Index % Count) + Count) % Count;
	CurrentTabIndex = NewIndex;

	if (WS_ContentSwitcher)
	{
		WS_ContentSwitcher->SetActiveWidgetIndex(NewIndex);
	}

	for (int32 i = 0; i < Tabs.Num(); ++i)
	{
		if (Tabs[i])
		{
			Tabs[i]->SetIsSelected(i == NewIndex, false);
		}
	}

	for (int32 i = 0; i < Contents.Num(); ++i)
	{
		if (Contents[i] && i != NewIndex)
		{
			Contents[i]->DeactivateWidget();
		}
	}
	if (Contents.IsValidIndex(NewIndex) && Contents[NewIndex])
	{
		Contents[NewIndex]->ActivateWidget();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
			{
				if (UCommonUIActionRouterBase* Router = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
				{
					Router->RefreshUIInputConfig();
				}
			}
		}));
	}
}

bool USettingsMenuWidget::NativeOnBackAction()
{
	ApplyAndClose();
	return true;
}
