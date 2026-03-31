// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStepResult.generated.h"

UENUM(BlueprintType)
enum class EActivityStepStatus : uint8
{
	// CriticalSuccess, // TODO Maybe ?
	Success,
	Fail,
	CriticalFail
};

USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityStepResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EActivityStepStatus Status = EActivityStepStatus::Success;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;
};
