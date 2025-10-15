// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StrategyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UStrategyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class POTIONPANIC_API IStrategyInterface
{
	GENERATED_BODY()
	
public:

	virtual void ExecuteStrategy(class UStationComponent* Station, TSubclassOf<AActor> InputItem, TSubclassOf<AActor>& OutputItem) = 0;

};
