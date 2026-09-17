// Fill out your copyright notice in the Description page of Project Settings.

#include "RoundChoiceWidget.h"
#include "AlchemyGameState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include <Engine/World.h>

DEFINE_LOG_CATEGORY_STATIC(MS_RoundChoiceWidget, Log, All);

URoundChoiceWidget::URoundChoiceWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Unlike the menus the base class is made for, this lives in the HUD from the start of the
	// level: activating on construction would open it at once and hand the input to it.
	bAutoActivate = false;

	// Activation is the switch: it shows the screen, deactivation collapses it again.
	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::Visible;
	bSetVisibilityOnDeactivated = true;
	DeactivatedVisibility = ESlateVisibility::Collapsed;

	// Where the gamepad lands when the screen opens.
	DesiredFocusWidgetName = TEXT("Choice1Button");
}

void URoundChoiceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Hidden until a choice opens, whatever the designer left it at.
	SetVisibility(ESlateVisibility::Collapsed);

	Choice1Button->OnClicked.AddDynamic(this, &ThisClass::OnChoice1Clicked);
	Choice2Button->OnClicked.AddDynamic(this, &ThisClass::OnChoice2Clicked);
	Choice3Button->OnClicked.AddDynamic(this, &ThisClass::OnChoice3Clicked);

	if (TryBindToGameState())
		return;

	// A client builds its HUD from its player controller, which can begin play before the game
	// state has replicated in. Binding to nothing here would leave this screen deaf for the
	// whole level: the host would pick, and the clients would never see the options.
	if (UWorld* World = GetWorld())
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
}

void URoundChoiceWidget::NativeDestruct()
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
		GameState->OnNextRoundChoiceStarted.RemoveDynamic(this, &ThisClass::OnChoiceStarted);
		GameState->OnNextRoundChosen.RemoveDynamic(this, &ThisClass::OnChoiceMade);
	}
}

bool URoundChoiceWidget::TryBindToGameState()
{
	UWorld* World = GetWorld();
	auto* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return false;

	GameState->OnNextRoundChoiceStarted.AddDynamic(this, &ThisClass::OnChoiceStarted);
	GameState->OnNextRoundChosen.AddDynamic(this, &ThisClass::OnChoiceMade);

	// A choice already open has no broadcast left to come: a widget built in the middle of it
	// still has to show the options, so it picks them up from the replicated state instead.
	if (GameState->IsChoosingNextRound())
		OnChoiceStarted(GameState->GetNextRoundChoices());

	return true;
}

void URoundChoiceWidget::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (!TryBindToGameState())
		return; // Some other game state arrived: keep waiting for ours.

	GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	GameStateSetHandle.Reset();
}

void URoundChoiceWidget::OnChoiceStarted(const TArray<int32>& RoundIndices)
{
	OfferedRounds = RoundIndices;

	if (OfferedRounds.Num() > 3)
	{
		UE_LOGFMT(MS_RoundChoiceWidget, Warning,
			"{0} rounds offered but this screen has three buttons: the extra ones cannot be picked.", OfferedRounds.Num());
	}

	SetUpChoice(0, Choice1Button, Choice1Text);
	SetUpChoice(1, Choice2Button, Choice2Text);
	SetUpChoice(2, Choice3Button, Choice3Text);

	// Activating is what switches the input to the menu and puts the focus on a button, so the
	// host can actually pick one.
	ActivateWidget();
}

void URoundChoiceWidget::SetUpChoice(int32 Choice, UButton* Button, UTextBlock* Text)
{
	const auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr;

	FRound Round;
	const bool bOffered = OfferedRounds.IsValidIndex(Choice)
		&& GameState && GameState->GetRound(OfferedRounds[Choice], Round);

	// Fewer than three rounds offered: the spare buttons go away rather than lead nowhere.
	Button->SetVisibility(bOffered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (!bOffered)
		return;

	Text->SetText(Round.RoundName);

	// Everyone sees the options; only the host can press them.
	Button->SetIsEnabled(GameState->CanChooseNextRound());
}

void URoundChoiceWidget::PickChoice(int32 Choice)
{
	if (!OfferedRounds.IsValidIndex(Choice))
		return;

	if (auto* GameState = GetWorld() ? GetWorld()->GetGameState<AAlchemyGameState>() : nullptr)
		GameState->ChooseNextRound(OfferedRounds[Choice]);
}

void URoundChoiceWidget::OnChoiceMade(int32 RoundIndex)
{
	OnRoundChosen(RoundIndex);
	OfferedRounds.Reset();
	DeactivateWidget();
}
