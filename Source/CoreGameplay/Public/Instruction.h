// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityAsset.h"
#include "Instruction.generated.h"

USTRUCT(BlueprintType)
struct COREGAMEPLAY_API FInstruction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	FPrimaryAssetId InputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	FPrimaryAssetId OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	TSoftObjectPtr<UActivityAsset> Activity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction", meta = (ClampMin = "0.0"))
	float ProcessingDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	bool bRequiresProximity = true;
};
