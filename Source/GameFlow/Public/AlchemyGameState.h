// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
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

	// Beware, this must be compared using `GetServerWorldTimeSeconds()`, not the local time.
	// You probably want to use `GetRoundRemainingTime()` instead.
	UFUNCTION(BlueprintCallable)
	float GetRoundEndTime() const { return RoundEndTime; }

	UFUNCTION(BlueprintCallable)
	float GetRoundRemainingTime() const;

private:

	UFUNCTION()
	void OnRep_SoftWorldData();
	
	void OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject);
		
	UFUNCTION()
	void OnRootRoundApplied();
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_SoftWorldData)
	TSoftObjectPtr<UWorldData> SoftWorldData;

	UPROPERTY(Transient)
	TObjectPtr<UWorldData> WorldData;

	UPROPERTY(Replicated)
	float RoundEndTime;
};
