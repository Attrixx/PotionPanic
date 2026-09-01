// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Order.h"
#include "Rounds/Round.h"
#include "AlchemyGameState.generated.h"

class UWorldData;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API AAlchemyGameState : public AGameStateBase
{
	GENERATED_BODY()

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	void SetWorldData(const TSoftObjectPtr<UWorldData>& NewWorldData);

	UFUNCTION(BlueprintCallable)
	float GetRoundTime() const;

	UFUNCTION(BlueprintCallable)
	float GetRoundRemainingTime() const;
	
	UFUNCTION(BlueprintCallable)
	const TArray<FOrder>& GetRoundOrders() const { return RoundOrders; }

	FOrderDelegate OnOrderChanged;

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
	float RoundStartTime;
	
	UPROPERTY(Replicated)
	float RoundEndTime;

	UPROPERTY(ReplicatedUsing=OnRep_RoundOrders)
	TArray<FOrder> RoundOrders;
	
	uint32 GenOrderId() { return OrderIdCounter++; }
	uint32 OrderIdCounter;
};
