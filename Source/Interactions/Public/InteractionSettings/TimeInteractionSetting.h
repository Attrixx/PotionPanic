// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionSettingBase.h"
#include "TimeInteractionSetting.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UTimeInteractionSetting : public UInteractionSettingBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SecondsToWait = 0.f;
};
