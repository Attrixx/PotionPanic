// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ScorePopupWidget.generated.h"

class UTextBlock;

/**
 * The "+64" that pops over a delivery. UScorePopupComponent fills it and times it; the Blueprint
 * lays out the bound text and, optionally, animates OnShown.
 */
UCLASS(Abstract)
class USERINTERFACES_API UScorePopupWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/** Writes the points in, "+64", then fires OnShown. */
	void Show(int32 Score);

protected:

	/** Optional hook for an animation, fired on every delivery, so also while a previous one still shows. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnShown(int32 Score);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
};
