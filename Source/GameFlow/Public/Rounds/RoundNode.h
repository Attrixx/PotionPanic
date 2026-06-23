// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoundContent.h"
#include "RoundNode.generated.h"

USTRUCT(BlueprintType)
struct GAMEFLOW_API FRoundNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RoundName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FRoundContent Content;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> ChildIndices;
};
