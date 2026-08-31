// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelProgressionTypes.h"
#include "LevelPreviewWidget.generated.h"

class UTextBlock;
class UBorder;

/**
 * 
 */
UCLASS()
class USERINTERFACES_API ULevelPreviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetLevelData(const FLevelData& InLevelData);

protected:

	void NativeConstruct() override;

	UFUNCTION()
	void HandleLevelDataChanged(const FLevelData& NewData);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelNumberText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> LevelImageBorder;

};
