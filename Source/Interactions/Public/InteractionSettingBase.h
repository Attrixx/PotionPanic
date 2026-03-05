// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InteractionSettingBase.generated.h"

class UInteractionBase;

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UInteractionSettingBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UInteractionBase> InteractionClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequiresPlayerInteraction = false;
};
