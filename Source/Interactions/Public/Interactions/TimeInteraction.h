// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionBase.h"
#include "TimeInteraction.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UTimeInteraction : public UInteractionBase
{
	GENERATED_BODY()
	
public:
	void Init(UInteractionSetting* Settings) override;
	void StartInteraction(const FInteractionContext& Context) override;
	
private:
	float SecondsToWait = 0.f;
};
