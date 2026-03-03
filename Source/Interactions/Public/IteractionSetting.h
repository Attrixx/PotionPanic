// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IteractionSetting.generated.h"

class UInteractionBase;

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UIteractionSetting : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInteractionBase> InteractionClass;
};
