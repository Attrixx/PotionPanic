// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/QuickTimeEventWidget.h"
#include "Components/Image.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

void UQuickTimeEventWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Image_InputKey->SetVisibility(ESlateVisibility::Hidden);
}

void UQuickTimeEventWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UQuickTimeEventWidget::ShowInputKey(const FGameplayTag& KeyTag)
{
	Image_InputKey->SetVisibility(ESlateVisibility::Visible);
	Image_InputKey->SetBrushFromTexture(InputKeyTextures[KeyTag]);
}