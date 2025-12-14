// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "QuickTimeEventWidget.generated.h"

class UImage;

UCLASS()
class POTIONPANIC_API UQuickTimeEventWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void NativeConstruct() override;
	void NativeDestruct() override;

	void StartQuickTimeEvent(const FGameplayTag& KeyTag, float Duration);
	void StopQuickTimeEvent();

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_InputKey;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_ShinyBorder;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Animation_Progress;

	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, TObjectPtr<UTexture2D>> InputKeyTextures;

	UPROPERTY(EditDefaultsOnly)
	float PerfectTimingWindow = 0.25f;

private:

	FTimerHandle TimerHandle_PerfectTiming;
	
};
