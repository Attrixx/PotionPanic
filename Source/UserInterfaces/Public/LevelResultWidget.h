// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "LevelResult.h"
#include "LevelResultWidget.generated.h"

class AGameStateBase;
class UButton;
class UTextBlock;

/**
 * End screen of a level. Everything is wired from here: the Blueprint only has to lay out the
 * bound texts and buttons. Fills itself in and activates when the level is over. Every machine
 * shows it; only the host's buttons are visible, being the only ones that do anything.
 */
UCLASS(Abstract)
class USERINTERFACES_API ULevelResultWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

public:

	ULevelResultWidget(const FObjectInitializer& ObjectInitializer);

protected:

	void NativeOnInitialized() override;
	void NativeDestruct() override;

	/**
	 * Optional hook, run on every machine once the bound widgets are filled and right before
	 * the widget activates: for anything to show beyond what is bound here.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ShowLevelResult(const FLevelResult& Result);

	/** Shows VictoryText or DefeatText. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OutcomeText;

	/** Shows the score against the target, "1250 / 1000". Optional. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReplayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;

	UPROPERTY(EditDefaultsOnly, Category = "Outcome")
	FText VictoryText = NSLOCTEXT("LevelResult", "Victory", "Victory");

	UPROPERTY(EditDefaultsOnly, Category = "Outcome")
	FText DefeatText = NSLOCTEXT("LevelResult", "Defeat", "Defeat");

private:

	/** @return False when the world has no AAlchemyGameState yet, nothing having been bound. */
	bool TryBindToGameState();

	/** Retries the binding when the world receives the game state this widget was waiting for. */
	void OnGameStateSet(AGameStateBase* NewGameState);

	UFUNCTION()
	void OnLevelComplete(const FLevelResult& Result);

	UFUNCTION()
	void OnReplayClicked();

	UFUNCTION()
	void OnReturnToLobbyClicked();

private:

	/** Valid only while waiting for the game state, so the wait can be dropped once it is over. */
	FDelegateHandle GameStateSetHandle;
};
