// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionSettingBase.h"
#include "IFTInteractionSetting.generated.h"

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UIFTInteractionSetting : public UInteractionSettingBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SecondsBeforeWindow = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WindowLengthSeconds = 1.f;
};
