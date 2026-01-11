// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Instruction.generated.h"

USTRUCT(BlueprintType)
struct COREGAMEPLAY_API FInstruction
{
	GENERATED_BODY()

	FName InputItem;
	FName OutputItem;
	FName Activity;
};
