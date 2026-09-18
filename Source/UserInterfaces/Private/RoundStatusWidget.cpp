// Fill out your copyright notice in the Description page of Project Settings.

#include "RoundStatusWidget.h"
#include "AlchemyGameState.h"
#include "Components/TextBlock.h"
#include <Engine/World.h>

void URoundStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TryBindToGameState())
		return;

	// A client builds its HUD from its player controller, which can begin play before the game
	// state has replicated in. Binding to nothing here would leave the round counter stuck.
	if (UWorld* World = GetWorld())
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
}

void URoundStatusWidget::NativeDestruct()
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
	{
		GameState->OnRoundStarted.RemoveDynamic(this, &ThisClass::OnRoundStarted);
		GameState->OnRoundEnded.RemoveDynamic(this, &ThisClass::OnRoundEnded);
		GameState->OnScoreChanged.RemoveDynamic(this, &ThisClass::OnScoreChanged);
	}
}

void URoundStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshTimeText();
}

bool URoundStatusWidget::TryBindToGameState()
{
	UWorld* World = GetWorld();
	auto* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return false;

	GameState->OnRoundStarted.AddDynamic(this, &ThisClass::OnRoundStarted);
	GameState->OnRoundEnded.AddDynamic(this, &ThisClass::OnRoundEnded);
	GameState->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);

	// A round already running has no start broadcast left to come: a widget built in the middle
	// of it reads the clock from the replicated state instead. Time left is the tell.
	bRoundRunning = GameState->GetRoundNumber() > 0 && GameState->GetRoundRemainingTime() > 0.f;
	RefreshRoundText();
	RefreshTimeText();
	RefreshScoreText();
	return true;
}

void URoundStatusWidget::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (!TryBindToGameState())
		return; // Some other game state arrived: keep waiting for ours.

	GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	GameStateSetHandle.Reset();
}

void URoundStatusWidget::OnRoundStarted(const FRound& Round)
{
	bRoundRunning = true;
	RefreshRoundText();
	RefreshTimeText();
}

void URoundStatusWidget::OnRoundEnded(const FRound& Round)
{
	bRoundRunning = false;
	RefreshTimeText();
}

void URoundStatusWidget::OnScoreChanged(int64 NewScore, int32 Delta)
{
	RefreshScoreText();
}

void URoundStatusWidget::RefreshScoreText()
{
	const auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return;

	ScoreText->SetText(FText::AsNumber(GameState->GetScore()));

	if (ScoreTargetText)
		ScoreTargetText->SetText(FText::AsNumber(GameState->GetScoreToSucceed()));
}

void URoundStatusWidget::RefreshRoundText()
{
	const auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return;

	RoundText->SetText(FText::Format(
		NSLOCTEXT("RoundStatus", "RoundOverRun", "Round {0}/{1}"),
		FText::AsNumber(GameState->GetRoundNumber()), FText::AsNumber(GameState->GetRunLength())));
}

void URoundStatusWidget::RefreshTimeText()
{
	const auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr;

	// Ceiling rather than floor: the display should read 0:01 for as long as any time is left,
	// and only show 0:00 once the round is truly over.
	const int32 Seconds = (GameState && bRoundRunning)
		? FMath::Max(0, FMath::CeilToInt32(GameState->GetRoundRemainingTime()))
		: 0;

	if (Seconds == LastShownSeconds)
		return;
	LastShownSeconds = Seconds;

	TimeText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Seconds / 60, Seconds % 60)));
}
