// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "LevelResult.h"
#include "LevelResultWidget.generated.h"

/**
 * End screen of a level: waits for the game state to report the run over, then hands the
 * outcome to Blueprint to display. Created hidden, it shows itself through ShowLevelResult.
 */
UCLASS(Abstract)
class USERINTERFACES_API ULevelResultWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:

	void NativeOnInitialized() override;
	void NativeDestruct() override;

	/**
	 * Fills the screen in and makes it visible. Runs on every machine, server and clients alike,
	 * once the last round of the level is over.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ShowLevelResult(const FLevelResult& Result);

private:

	UFUNCTION()
	void OnLevelComplete(const FLevelResult& Result);
};
