// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelSelectorUIInterface.h"
#include "LevelNumberWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class USERINTERFACES_API ULevelNumberWidget : public UUserWidget, public ILevelSelectorUIInterface
{
	GENERATED_BODY()

protected:

	void NativeConstruct() override;

	void SetLevelNumber_Implementation(int32 LevelNumber) override;
	void OnLevelUnlocked_Implementation() override;

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LevelNumberImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> PrintNumber;
	
};
