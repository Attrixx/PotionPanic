#include "SettingsMenuWidget.h"
#include "VideoSettingsWidget.h"
#include "AudioSettingsWidget.h"
#include "ControlsSettingsWidget.h"
#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Input/CommonUIActionRouterBase.h"
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
	SelectTab(0);
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
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
			{
				if (UCommonUIActionRouterBase* Router = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
				{
					Router->RefreshUIInputConfig();
				}
			}
		}), 0.01f, false);
	}
}

bool USettingsMenuWidget::NativeOnBackAction()
{
	ApplyAndClose();
	return true;
}

FReply USettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Gamepad_RightShoulder)
	{
		SelectTab(CurrentTabIndex + 1);
		return FReply::Handled();
	}
	if (Key == EKeys::Gamepad_LeftShoulder)
	{
		SelectTab(CurrentTabIndex - 1);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
