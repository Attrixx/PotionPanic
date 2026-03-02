// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityAsset.h"
#include "Instruction.generated.h"

USTRUCT(BlueprintType)
struct STATIONS_API FInstruction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	FPrimaryAssetId InputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction", meta = (ClampMin = "1"))
	int32 InputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	FPrimaryAssetId OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction", meta = (ClampMin = "1"))
	int32 OutputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	TSoftObjectPtr<UActivityAsset> Activity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction", meta = (ClampMin = "0.0"))
	float ProcessingDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction")
	bool bRequiresProximity = true;

	/** If true, the held input is consumed/destroyed when processing succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction|Success")
	bool bConsumeInputOnSuccess = true;

	/** If true, output item(s) are spawned when processing succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction|Success")
	bool bProduceOutputOnSuccess = true;

	/** If true, consumed input should be removed when processing fails/cancels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction|Failure")
	bool bConsumeInputOnFailure = false;

	/** Optional item spawned when processing fails/cancels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction|Failure")
	FPrimaryAssetId FailureOutputItem;

	/** Quantity spawned on failure when FailureOutputItem is valid. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instruction|Failure", meta = (ClampMin = "1"))
	int32 FailureOutputQuantity = 1;
};
