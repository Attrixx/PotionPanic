// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/QuickTimeEventWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

void UQuickTimeEventWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Image_InputKey->SetVisibility(ESlateVisibility::Hidden);
	Image_ShinyBorder->SetVisibility(ESlateVisibility::Hidden);
}

void UQuickTimeEventWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UQuickTimeEventWidget::StartQuickTimeEvent(const FGameplayTag& KeyTag, float Duration)
{
	Image_InputKey->SetVisibility(ESlateVisibility::Visible);
	Image_ShinyBorder->SetVisibility(ESlateVisibility::Hidden);
	Image_InputKey->SetBrushFromTexture(InputKeyTextures[KeyTag]);
	PlayAnimation(Animation_Progress, 0.f, 1, EUMGSequencePlayMode::Forward, Animation_Progress->GetEndTime() / Duration);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_PerfectTiming,
		[this]()
		{
			Image_ShinyBorder->SetVisibility(ESlateVisibility::Visible);
		},
		Duration - (PerfectTimingWindow * Duration),
		false
	);
}

void UQuickTimeEventWidget::StopQuickTimeEvent()
{
	Image_InputKey->SetVisibility(ESlateVisibility::Hidden);
	Image_ShinyBorder->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PerfectTiming);
}
