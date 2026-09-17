// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RoundStatusWidget.generated.h"

class AGameStateBase;
class UTextBlock;
struct FRound;

/**
 * HUD element showing the round the players are in and the time it has left. Everything is
 * wired from here: the Blueprint only has to lay out the two bound texts. Counts down on every
 * machine from the replicated round timing, so the clients read the same clock as the host.
 */
UCLASS(Abstract)
class USERINTERFACES_API URoundStatusWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:

	void NativeOnInitialized() override;
	void NativeDestruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Time left in the round, "M:SS". Frozen at 0:00 between rounds. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	/** Position of the round in the run over the run's length, "1/3". */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoundText;

private:

	/** @return False when the world has no AAlchemyGameState yet, nothing having been bound. */
	bool TryBindToGameState();

	/** Retries the binding when the world receives the game state this widget was waiting for. */
	void OnGameStateSet(AGameStateBase* NewGameState);

	UFUNCTION()
	void OnRoundStarted(const FRound& Round);

	UFUNCTION()
	void OnRoundEnded(const FRound& Round);

	void RefreshRoundText();
	void RefreshTimeText();

private:

	/** Only counts down between OnRoundStarted and OnRoundEnded; shows 0:00 otherwise. */
	bool bRoundRunning = false;

	/** Whole seconds last written, so the text is only rebuilt when the display changes. */
	int32 LastShownSeconds = -1;

	/** Valid only while waiting for the game state, so the wait can be dropped once it is over. */
	FDelegateHandle GameStateSetHandle;
};
