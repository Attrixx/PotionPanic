// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RoundLoader.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnRoundAppliedDelegate);

struct FRound;
struct FStreamableHandle;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API URoundLoader : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Loads the round assets, then makes the round current: the station layout and the recipe pool
	 * are reset to their default state before the round layers and recipes are applied.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static URoundLoader* LoadAndApplyRound(UObject* WorldContextObject, const FRound& Round, FOnRoundAppliedDelegate OnComplete);
		
	/** Drops the pending load: neither the round nor the completion callback is applied. */
	UFUNCTION(BlueprintCallable)
	void Cancel();

	/** True while the load is in flight, so still worth cancelling. */
	UFUNCTION(BlueprintPure)
	bool IsPending() const { return bPending; }

private:
	
	void StartLoading(UObject* WorldContextObject, const FRound& Round, FOnRoundAppliedDelegate OnComplete);
	void OnAssetsLoaded(TWeakObjectPtr<> WeakContext, FRound Round, FOnRoundAppliedDelegate OnComplete);
	
	TSharedPtr<FStreamableHandle> StreamableHandle;

	/** Cleared by whichever of the completion and the cancellation happens first. */
	bool bPending = true;
};
