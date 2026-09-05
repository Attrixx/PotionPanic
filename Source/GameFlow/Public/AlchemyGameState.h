// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ItemOrder.h"
#include "LevelResult.h"
#include "Rounds/Round.h"
#include "Engine/TimerHandle.h"
#include "AlchemyGameState.generated.h"

class UWorldData;
class URoundLoader;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRoundDelegate, const FRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelCompleteDelegate, const FLevelResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreDelegate, int64, NewScore, int32, Delta);

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

	/** Points gathered since the level started, across every round played so far. */
	UFUNCTION(BlueprintCallable)
	int64 GetScore() const { return Score; }

	/** Points the level asks for to be won. Zero until the world data is there. */
	UFUNCTION(BlueprintCallable)
	int64 GetScoreToSucceed() const;

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

	/** Fires on every machine whenever a delivery moves the score, Delta being what it added. */
	UPROPERTY(BlueprintAssignable)
	FScoreDelegate OnScoreChanged;

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
	 * Places pending orders and cancels expired ones, based on the current round time, then ends
	 * the round once nothing is left to resolve. Server only: the clients learn every transition
	 * through OnRep_RoundOrders instead, so that RoundOrders never differs from the authority's.
	 */
	void UpdateOrders();

	/** Moves every pending order Shift seconds earlier, preserving the spacing between them. */
	void ShiftPendingOrders(double Shift);

	/** Reports every remaining order as deleted, drops them and stops ticking. Server only. */
	void EndRound();
	void CancelOngoingStationActivities();

	/** Applies NewState and notifies local listeners, OnRep_RoundOrders doing it for the clients. */
	void SetOrderState(FItemOrder& Order, EOrderState NewState);

	/**
	 * What completing Order right now is worth: MaxOrderScore when it is delivered the instant it
	 * is placed, MinOrderScore on its deadline, scaling with the time left in between.
	 */
	int32 ScoreForOrder(const FItemOrder& Order, double RemainingTime) const;

	/** Adds Delta to the score and notifies local listeners, OnRep_Score doing it for the clients. */
	void AddScore(int32 Delta);

	UFUNCTION()
	void OnRep_Score(int64 OldScore);

	UFUNCTION()
	void OnRep_RoundOrders(const TArray<FItemOrder>& OldRoundOrders);

	/**
	 * Relays OnRoundStarted/OnRoundEnded/OnLevelComplete to every client: as plain
	 * BlueprintAssignable delegates they only fire locally, and StartRound/EndRound only ever
	 * run on the server. The round is resent as an index rather than as an FRound since every
	 * machine can already resolve it from WorldData.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRoundStarted(int32 RoundIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRoundEnded(int32 RoundIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnLevelComplete(const FLevelResult& Result);

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

	/** Points gathered since the level started. Replicated so the HUD can count along live. */
	UPROPERTY(ReplicatedUsing=OnRep_Score)
	int64 Score = 0;

	/** Level-wide order tallies, kept on the server and shipped out with the level result. */
	int32 LevelCompletedOrders = 0;
	int32 LevelFailedOrders = 0;

	int32 GenOrderId() { return OrderIdCounter++; }
	int32 OrderIdCounter = 0;
};
