// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionBase.h"
#include "InteractionTest.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class INTERACTIONS_API UInteractionTest : public UInteractionBase
{
	GENERATED_BODY()
	
public:	
	void Init(UInteractionSettingBase* Settings) override;
	void StartInteraction(const FInteractionContext& Context) override;
};
