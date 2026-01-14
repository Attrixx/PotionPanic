// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityAsset.h"
#include "Instruction.generated.h"

class UItemAsset;

USTRUCT(BlueprintType)
struct COREGAMEPLAY_API FInstruction
{
	GENERATED_BODY()

	FPrimaryAssetId InputItem;
	FPrimaryAssetId OutputItem;
	TObjectPtr<UActivityAsset> Activity;
};
