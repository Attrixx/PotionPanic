// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ScorePopupComponent.generated.h"

class AGameStateBase;
class USoundBase;

/**
 * Pops the points earned over its owner whenever an order is delivered there, on every machine.
 * Add it to the delivery station and set Widget Class to a UScorePopupWidget: it stays hidden
 * until a delivery, shows for DisplayDuration, then hides again.
 */
UCLASS(ClassGroup = "UI", meta = (BlueprintSpawnableComponent))
class USERINTERFACES_API UScorePopupComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UScorePopupComponent();

protected:

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** How long the points stay up. Another delivery in the meantime restarts the clock. */
	UPROPERTY(EditAnywhere, Category = "Score Popup", meta = (ClampMin = 0.1))
	float DisplayDuration = 1.5f;

	/** Played at this component's location on every delivery, on every machine. Optional. */
	UPROPERTY(EditAnywhere, Category = "Score Popup")
	TObjectPtr<USoundBase> DeliverySound;

private:

	/** @return False when the world has no AAlchemyGameState yet, nothing having been bound. */
	bool TryBindToGameState();

	/** Retries the binding when the world receives the game state this component was waiting for. */
	void OnGameStateSet(AGameStateBase* NewGameState);

	UFUNCTION()
	void OnOrderDelivered(AActor* DeliveredAt, int32 Score);

	void Hide();

private:

	FTimerHandle HideHandle;

	/** Valid only while waiting for the game state, so the wait can be dropped once it is over. */
	FDelegateHandle GameStateSetHandle;
};
