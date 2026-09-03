// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Order.h"
#include "Rounds/Round.h"
#include "AlchemyGameState.generated.h"

class UWorldData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEndedDelegate);

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
	const TArray<FOrder>& GetRoundOrders() const { return RoundOrders; }

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
	FOnRoundEndedDelegate OnRoundEnded;

private:

	UFUNCTION()
	void OnRep_SoftWorldData();
	
	void OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject);
		
	void SetCurrentRound(int32 Index);
	const FRound* GetCurrentRound() const;
	
	UFUNCTION()
	void OnCurrentRoundApplied();
	
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

	/** Applies NewState and notifies local listeners, OnRep_RoundOrders doing it for the clients. */
	void SetOrderState(FOrder& Order, EOrderState NewState);
	
	UFUNCTION()
	void OnRep_RoundOrders(const TArray<FOrder>& OldRoundOrders);
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_SoftWorldData)
	TSoftObjectPtr<UWorldData> SoftWorldData;

	UPROPERTY(Transient)
	TObjectPtr<UWorldData> WorldData;

	UPROPERTY(Replicated)
	int32 CurrentRound = 0;
	
	UPROPERTY(Replicated)
	float RoundStartTime = 0.f;

	UPROPERTY(Replicated)
	float RoundEndTime = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_RoundOrders)
	TArray<FOrder> RoundOrders;
	
	uint32 GenOrderId() { return OrderIdCounter++; }
	uint32 OrderIdCounter;
};
