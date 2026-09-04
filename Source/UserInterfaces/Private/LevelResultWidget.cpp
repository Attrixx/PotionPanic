// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelResultWidget.h"
#include "AlchemyGameState.h"
#include <Engine/World.h>

void ULevelResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TryBindToGameState())
		return;

	// A client builds its HUD from its player controller, which can begin play before the game
	// state has replicated in. Binding to nothing here would cost the whole end screen: the level
	// would finish and this widget would never hear about it.
	if (UWorld* World = GetWorld())
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
}

void ULevelResultWidget::NativeDestruct()
{
	Super::NativeDestruct();

	UWorld* World = GetWorld();
	if (!World)
		return;

	if (GameStateSetHandle.IsValid())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
		GameStateSetHandle.Reset();
	}

	if (auto* GameState = World->GetGameState<AAlchemyGameState>())
		GameState->OnLevelComplete.RemoveDynamic(this, &ThisClass::OnLevelComplete);
}

bool ULevelResultWidget::TryBindToGameState()
{
	UWorld* World = GetWorld();
	auto* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return false;

	// Nothing to replay: a level result is a one-shot event, and the level cannot already be over
	// by the time the HUD that shows its outcome is built.
	GameState->OnLevelComplete.AddDynamic(this, &ThisClass::OnLevelComplete);
	return true;
}

void ULevelResultWidget::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (!TryBindToGameState())
		return; // Some other game state arrived: keep waiting for ours.

	GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	GameStateSetHandle.Reset();
}

void ULevelResultWidget::OnLevelComplete(const FLevelResult& Result)
{
	ShowLevelResult(Result);
}
