// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ItemOrder.h"
#include "Rounds/Round.h"
#include "Engine/TimerHandle.h"
#include "AlchemyGameState.generated.h"

class UWorldData;
class URoundLoader;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRoundDelegate, const FRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelCompleteDelegate);

/**
 * 
 */
UCLASS()
class GAMEFLOW_API AAlchemyGameState : public AGameStateBase
{
	GENERATED_BODY()

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	AAlchemyGameState();

	void Tick(float DeltaSeconds) override;

	void SetWorldData(const TSoftObjectPtr<UWorldData>& NewWorldData);

	UFUNCTION(BlueprintCallable)
	float GetRoundTime() const;

	UFUNCTION(BlueprintCallable)
	float GetRoundRemainingTime() const;
	
	UFUNCTION(BlueprintCallable)
	const TArray<FItemOrder>& GetRoundOrders() const { return RoundOrders; }

	/**
	 * Completes the placed order expiring the soonest among those asking for the delivered item.
	 * Server only.
	 * @param ItemAsset The object handed over, expected to be an AItemActor.
	 * @return True if the item was delivered. False when no placed order is waiting for this item.
	 */
	UFUNCTION(BlueprintCallable)
	bool DeliverOrder(UItemAsset* ItemAsset);

	FOrderDelegate OnOrderChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundDelegate OnRoundStarted;
	
	UPROPERTY(BlueprintAssignable)
	FRoundDelegate OnRoundEnded;
	
	UPROPERTY(BlueprintAssignable)
	FOnLevelCompleteDelegate OnLevelComplete;

private:

	UFUNCTION()
	void OnRep_SoftWorldData();
	
	void OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject);
		
	void SetCurrentRound(int32 Index);
	const FRound* GetCurrentRound() const;
	
	UFUNCTION()
	void OnCurrentRoundApplied();
	
	/** Drops the round load and the round start still pending, if any. Server only. */
	void CancelPendingRoundStart();

	/**
	 * True when every client the server knows about reported this world as loaded.
	 * A player still travelling in has no controller here yet and cannot be waited on, so this
	 * holds the round back for the clients already connected, not for an expected player count.
	 */
	bool AreAllPlayersReady();

	/** Starts the round once the players are ready, or once the wait times out. Server only. */
	void TryStartRound();

	void CreateOrders();
	void StartRound();

	/**
	 * Places pending orders and cancels expired ones, based on the current round time.
	 * Runs on clients too: they reach the same transitions from replicated data, and
	 * OnRep_RoundOrders corrects them whenever they drift. Only the server ends the round.
	 */
	void UpdateOrders();

	/** Moves every pending order Shift seconds earlier, preserving the spacing between them. */
	void ShiftPendingOrders(double Shift);

	/** Reports every remaining order as deleted, drops them and stops ticking. Server only. */
	void EndRound();
	void CancelOngoingStationActivities();

	/** Applies NewState and notifies local listeners, OnRep_RoundOrders doing it for the clients. */
	void SetOrderState(FItemOrder& Order, EOrderState NewState);
	
	UFUNCTION()
	void OnRep_RoundOrders(const TArray<FItemOrder>& OldRoundOrders);
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_SoftWorldData)
	TSoftObjectPtr<UWorldData> SoftWorldData;

	UPROPERTY(Transient)
	TObjectPtr<UWorldData> WorldData;

	/** Round load in flight. Null as soon as it completed or was cancelled. */
	UPROPERTY(Transient)
	TObjectPtr<URoundLoader> RoundLoader;

	/**
	 * How long the server waits for the clients to be ready before starting the round anyway.
	 * A client that never reports in must not hold the whole session hostage. Zero starts at once.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Round", meta = (ClampMin = 0))
	float MaxRoundStartWaitTime = 15.f;

	/** How often the readiness of the clients is checked while waiting to start a round. */
	UPROPERTY(EditDefaultsOnly, Category = "Round", meta = (ClampMin = 0.01))
	float RoundStartWaitPollInterval = 0.25f;

	FTimerHandle RoundStartWaitHandle;
	double RoundStartWaitDeadline = 0.0;

	UPROPERTY(Replicated)
	int32 CurrentRound = 0;
	
	UPROPERTY(Replicated)
	float RoundStartTime = 0.f;

	UPROPERTY(Replicated)
	float RoundEndTime = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_RoundOrders)
	TArray<FItemOrder> RoundOrders;
	
	uint32 GenOrderId() { return OrderIdCounter++; }
	uint32 OrderIdCounter;
};
