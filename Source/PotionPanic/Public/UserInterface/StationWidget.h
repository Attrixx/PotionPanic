// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StationWidget.generated.h"

class UImage;
class UOverlay;
class UProgressBar;

UCLASS()
class POTIONPANIC_API UStationWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void NativeConstruct() override;
	void NativeDestruct() override;

	void ShowInteractKey();
	void HideInteractKey();

	void ShowAnimatedProgress(float Duration, bool bAutoHide = true);
	void ShowProgress(float Progress);
	void HideProgress();

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_InputKey;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_ProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Texture_InteractKey;

	UPROPERTY(EditDefaultsOnly)
	int32 ProgressBarUpdateRate = 60;
	
};
