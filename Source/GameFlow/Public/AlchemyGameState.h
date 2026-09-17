// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ItemOrder.h"
#include "LevelResult.h"
#include "Rounds/Round.h"
#include "Rounds/RoundLoader.h"
#include "Engine/TimerHandle.h"
#include "AlchemyGameState.generated.h"

class UWorldData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRoundDelegate, const FRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelCompleteDelegate, const FLevelResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreDelegate, int64, NewScore, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNextRoundChoiceDelegate, const TArray<int32>&, RoundIndices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRoundIndexDelegate, int32, RoundIndex);

/**
 * 
 */
UCLASS()
class GAMEFLOW_API AAlchemyGameState : public AGameStateBase
{
	GENERATED_BODY()

	// Development cheats reach the round state from outside rather than widening this class's API.
	friend struct FAlchemyGameStateCheats;

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

	/** @return False when RoundIndex names no round of this world, OutRound being left untouched. */
	UFUNCTION(BlueprintCallable)
	bool GetRound(int32 RoundIndex, FRound& OutRound) const;

	/** Rounds the host is currently picking the next one from. Empty outside a choice. */
	UFUNCTION(BlueprintCallable)
	const TArray<int32>& GetNextRoundChoices() const { return NextRoundChoices; }

	UFUNCTION(BlueprintCallable)
	bool IsChoosingNextRound() const { return !NextRoundChoices.IsEmpty(); }

	/** True on the host while a choice is open: the only machine allowed to make it. */
	UFUNCTION(BlueprintCallable)
	bool CanChooseNextRound() const;

	/**
	 * Picks the round to play next among GetNextRoundChoices() and starts loading it. Host only:
	 * the clients follow the choice through OnNextRoundChoiceStarted and OnNextRoundChosen but
	 * cannot make it, a call from one of them is refused.
	 * @return False when no choice is open, RoundIndex is not one of the offered rounds, or this
	 * machine is a client.
	 */
	UFUNCTION(BlueprintCallable)
	bool ChooseNextRound(int32 RoundIndex);

	/** True on the host once the level is over: the only machine allowed to leave it. */
	UFUNCTION(BlueprintCallable)
	bool CanLeaveLevel() const;

	/**
	 * Plays this level again from its first round, taking every client along. Host only, and
	 * only once the level is over.
	 * @return False when refused, or when the travel could not start.
	 */
	UFUNCTION(BlueprintCallable)
	bool ReplayLevel();

	/**
	 * Brings the whole party back to the game mode's LobbyLevel, where another level can be
	 * picked. Host only, and only once the level is over.
	 * @return False when refused, when the game mode sets no lobby, or when the travel could not start.
	 */
	UFUNCTION(BlueprintCallable)
	bool ReturnToLobby();

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

	/**
	 * Fires on every machine when a round ends with several rounds able to follow it, carrying
	 * their indices. The host is then expected to call ChooseNextRound; the others only watch.
	 */
	UPROPERTY(BlueprintAssignable)
	FNextRoundChoiceDelegate OnNextRoundChoiceStarted;

	/** Fires on every machine once the host has picked, before that round begins loading. */
	UPROPERTY(BlueprintAssignable)
	FRoundIndexDelegate OnNextRoundChosen;

private:

	UFUNCTION()
	void OnRep_SoftWorldData();
	
	void OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject);
		
	void SetCurrentRound(int32 Index);
	const FRound* GetCurrentRound() const;

	UFUNCTION()
	void OnRep_CurrentRound();

	/** Streams the current round in and applies it locally, without starting anything. */
	void ApplyCurrentRoundLocally();

	/** Kicks off the load of Round, reporting to OnApplied once it has been applied. */
	void StartRoundLoad(const FRound& Round, FOnRoundAppliedDelegate OnApplied);

	UFUNCTION()
	void OnCurrentRoundApplied();

	/**
	 * Client counterpart of OnCurrentRoundApplied: nothing to start, but the finished loader
	 * still has to be let go, or the next round's CancelPendingRoundStart reports it in flight.
	 */
	UFUNCTION()
	void OnCurrentRoundAppliedLocally();

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

	/**
	 * Reports every remaining order as deleted, drops them and stops ticking, then moves on: to
	 * the level result when nothing follows, straight to the next round when only one does, or
	 * to a choice put to the host when several do. Server only.
	 */
	void EndRound();
	void CancelOngoingStationActivities();

	/** Opens the choice of the next round among Choices and tells every machine. Server only. */
	void BeginNextRoundChoice(const TArray<int32>& Choices);

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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnNextRoundChoiceStarted(const TArray<int32>& Choices);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnNextRoundChosen(int32 RoundIndex);

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

	/** Set by the last round ending. Server only: it gates the way out of the level. */
	bool bLevelOver = false;

	FTimerHandle RoundStartWaitHandle;
	double RoundStartWaitDeadline = 0.0;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentRound)
	int32 CurrentRound = 0;

	/**
	 * Rounds offered to the host at the end of the current one. Non-empty only while the choice
	 * is open; replicated so a HUD built in the middle of it can still show the options.
	 */
	UPROPERTY(Replicated)
	TArray<int32> NextRoundChoices;

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
