// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelResultWidget.h"
#include "AlchemyGameState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include <Engine/World.h>

ULevelResultWidget::ULevelResultWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Unlike the menus the base class is made for, this lives in the HUD from the start of the
	// level: activating on construction would open it at once and hand the input to it.
	bAutoActivate = false;

	// Activation is the switch: it shows the screen. Nothing deactivates it, the level is over.
	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::Visible;

	// Where the gamepad lands when the screen opens.
	DesiredFocusWidgetName = TEXT("ReplayButton");
}

void ULevelResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Hidden until the level is over, whatever the designer left it at.
	SetVisibility(ESlateVisibility::Collapsed);

	ReplayButton->OnClicked.AddDynamic(this, &ThisClass::OnReplayClicked);
	ReturnToLobbyButton->OnClicked.AddDynamic(this, &ThisClass::OnReturnToLobbyClicked);

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
	OutcomeText->SetText(Result.bSucceeded ? VictoryText : DefeatText);

	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(
			NSLOCTEXT("LevelResult", "ScoreOverTarget", "{0} / {1}"),
			FText::AsNumber(Result.Score), FText::AsNumber(Result.ScoreToSucceed)));
	}

	// Everyone sees the outcome; only the host gets the way out.
	const auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr;
	const ESlateVisibility ButtonVisibility = GameState && GameState->CanLeaveLevel()
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	ReplayButton->SetVisibility(ButtonVisibility);
	ReturnToLobbyButton->SetVisibility(ButtonVisibility);

	ShowLevelResult(Result);

	// Activating is what switches the input to the menu and puts the focus on a button, so the
	// host can actually press one.
	ActivateWidget();
}

void ULevelResultWidget::OnReplayClicked()
{
	if (auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr)
		GameState->ReplayLevel();
}

void ULevelResultWidget::OnReturnToLobbyClicked()
{
	if (auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr)
		GameState->ReturnToLobby();
}
