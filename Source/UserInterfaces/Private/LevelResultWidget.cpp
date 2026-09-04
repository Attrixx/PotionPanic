// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelResultWidget.h"
#include "AlchemyGameState.h"

DEFINE_LOG_CATEGORY_STATIC(MS_LevelResultWidget, Log, All);

void ULevelResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	if (!GameState)
	{
		// A HUD built before the game state replicated in would never hear the level end.
		UE_LOGFMT(MS_LevelResultWidget, Error, "No AAlchemyGameState yet: this end screen will never show.");
		return;
	}

	GameState->OnLevelComplete.AddDynamic(this, &ThisClass::OnLevelComplete);
}

void ULevelResultWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>())
		GameState->OnLevelComplete.RemoveDynamic(this, &ThisClass::OnLevelComplete);
}

void ULevelResultWidget::OnLevelComplete(const FLevelResult& Result)
{
	ShowLevelResult(Result);
}
