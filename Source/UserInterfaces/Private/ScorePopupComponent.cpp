// Fill out your copyright notice in the Description page of Project Settings.

#include "ScorePopupComponent.h"
#include "ScorePopupWidget.h"
#include "AlchemyGameState.h"
#include <Engine/World.h>
#include <TimerManager.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ScorePopupComponent, Log, All);

UScorePopupComponent::UScorePopupComponent()
{
	// Screen space keeps the text crisp and facing the camera whatever the station's angle.
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetHiddenInGame(true);
}

void UScorePopupComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TryBindToGameState())
		return;

	// On a client the station can begin play before the game state has replicated in.
	if (UWorld* World = GetWorld())
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
}

void UScorePopupComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideHandle);

		if (GameStateSetHandle.IsValid())
		{
			World->GameStateSetEvent.Remove(GameStateSetHandle);
			GameStateSetHandle.Reset();
		}

		if (auto* GameState = World->GetGameState<AAlchemyGameState>())
			GameState->OnOrderDelivered.RemoveDynamic(this, &ThisClass::OnOrderDelivered);
	}

	Super::EndPlay(EndPlayReason);
}

bool UScorePopupComponent::TryBindToGameState()
{
	UWorld* World = GetWorld();
	auto* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return false;

	GameState->OnOrderDelivered.AddDynamic(this, &ThisClass::OnOrderDelivered);
	return true;
}

void UScorePopupComponent::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (!TryBindToGameState())
		return; // Some other game state arrived: keep waiting for ours.

	GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	GameStateSetHandle.Reset();
}

void UScorePopupComponent::OnOrderDelivered(AActor* DeliveredAt, int32 Score)
{
	// Every popup in the level hears every delivery: only the one on the right station reacts.
	if (DeliveredAt != GetOwner())
		return;

	auto* Popup = Cast<UScorePopupWidget>(GetUserWidgetObject());
	if (!Popup)
	{
		UE_LOGFMT(MS_ScorePopupComponent, Error,
			"{0} has no UScorePopupWidget: set its Widget Class to one, nothing can be shown.", GetOwner()->GetName());
		return;
	}

	Popup->Show(Score);
	SetHiddenInGame(false);

	// A second delivery while the first still shows just pushes the hiding back.
	GetWorld()->GetTimerManager().SetTimer(HideHandle, this, &ThisClass::Hide, DisplayDuration, false);
}

void UScorePopupComponent::Hide()
{
	SetHiddenInGame(true);
}
