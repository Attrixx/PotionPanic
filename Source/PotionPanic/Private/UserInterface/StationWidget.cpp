// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/StationWidget.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"

void UStationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideInteractKey();
	HideProgress();

	if (Texture_InteractKey)
	{
		Image_InputKey->SetBrushFromTexture(Texture_InteractKey);
	}
}

void UStationWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UStationWidget::ShowInteractKey()
{
	Image_InputKey->SetVisibility(ESlateVisibility::Visible);
}

void UStationWidget::HideInteractKey()
{
	Image_InputKey->SetVisibility(ESlateVisibility::Hidden);
}

void UStationWidget::ShowAnimatedProgress(float Duration, bool bAutoHide)
{
	Overlay_ProgressBar->SetVisibility(ESlateVisibility::Visible);
	ProgressBar->SetPercent(0.0f);
	float UpdateInterval = 1.0f / ProgressBarUpdateRate;
	int32 TotalUpdates = FMath::CeilToInt(Duration / UpdateInterval);
	for (int32 i = 1; i <= TotalUpdates; ++i)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, i, TotalUpdates, bAutoHide]()
		{
			float Progress = static_cast<float>(i) / static_cast<float>(TotalUpdates);
			ProgressBar->SetPercent(Progress);
			if (bAutoHide && Progress >= 1.0f)
			{
				HideProgress();
			}
		}, i * UpdateInterval, false);
	}
}

void UStationWidget::ShowProgress(float Progress)
{
	Overlay_ProgressBar->SetVisibility(ESlateVisibility::Visible);
	ProgressBar->SetPercent(Progress);
}

void UStationWidget::HideProgress()
{
	Overlay_ProgressBar->SetVisibility(ESlateVisibility::Hidden);
}
