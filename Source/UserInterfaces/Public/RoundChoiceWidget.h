// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "RoundChoiceWidget.generated.h"

class AGameStateBase;
class UButton;
class UTextBlock;

/**
 * Screen where the host picks the round to play next, between rounds. Everything is wired from
 * here: the Blueprint only has to lay out three buttons and three texts under the bound names.
 * Activates itself when a choice opens and deactivates once the host has picked. Every machine
 * shows it; only the host's buttons are enabled.
 */
UCLASS(Abstract)
class USERINTERFACES_API URoundChoiceWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

public:

	URoundChoiceWidget(const FObjectInitializer& ObjectInitializer);

protected:

	void NativeOnInitialized() override;
	void NativeDestruct() override;

	/**
	 * Optional hook, run on every machine once the host has picked and right before the widget
	 * deactivates: the place to highlight the chosen option for the players who only watched.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnRoundChosen(int32 RoundIndex);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Choice1Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Choice2Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Choice3Button;

	/** Shows the name of the round each button leads to. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Choice1Text;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Choice2Text;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Choice3Text;

private:

	/** @return False when the world has no AAlchemyGameState yet, nothing having been bound. */
	bool TryBindToGameState();

	/** Retries the binding when the world receives the game state this widget was waiting for. */
	void OnGameStateSet(AGameStateBase* NewGameState);

	UFUNCTION()
	void OnChoiceStarted(const TArray<int32>& RoundIndices);

	UFUNCTION()
	void OnChoiceMade(int32 RoundIndex);

	/** Fills one button from the round at Choice in OfferedRounds, or hides it when there is none. */
	void SetUpChoice(int32 Choice, UButton* Button, UTextBlock* Text);

	/** Picks the round behind Choice. Host only, refused elsewhere. */
	void PickChoice(int32 Choice);

	UFUNCTION() void OnChoice1Clicked() { PickChoice(0); }
	UFUNCTION() void OnChoice2Clicked() { PickChoice(1); }
	UFUNCTION() void OnChoice3Clicked() { PickChoice(2); }

private:

	/** Round index behind each button while a choice is open. */
	TArray<int32> OfferedRounds;

	/** Valid only while waiting for the game state, so the wait can be dropped once it is over. */
	FDelegateHandle GameStateSetHandle;
};
