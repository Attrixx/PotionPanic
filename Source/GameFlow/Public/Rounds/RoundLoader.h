// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RoundLoader.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnRoundAppliedDelegate);

struct FRoundContent;
struct FStreamableHandle;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API URoundLoader : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static URoundLoader* LoadAndApplyRound(UObject* WorldContextObject, const FRoundContent& RoundContent, FOnRoundAppliedDelegate OnComplete);

private:
	
	void StartLoading(UObject* WorldContextObject, const FRoundContent& RoundContent, FOnRoundAppliedDelegate OnComplete);
	void OnAssetsLoaded(TWeakObjectPtr<UObject> WeakContext, FRoundContent RoundContent, FOnRoundAppliedDelegate OnComplete);
	
	TSharedPtr<FStreamableHandle> StreamableHandle;
};
