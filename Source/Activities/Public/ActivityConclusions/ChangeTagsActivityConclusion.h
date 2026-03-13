// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "GameplayTagContainer.h"
#include "ChangeTagsActivityConclusion.generated.h"

UENUM(BlueprintType)
enum class EChangeTagsMethod : uint8
{
	DoNothing = 0,
	SetTags,
	AddTags,
	RemoveTags,
};

/**
 * 
 */
UCLASS(DisplayName = "Change Tags")
class ACTIVITIES_API UChangeTagsActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()
	
	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;
	
	UPROPERTY(EditAnywhere, Category="")
	EChangeTagsMethod OnSuccessMethod = EChangeTagsMethod::DoNothing;
	
	UPROPERTY(EditAnywhere, Category="", meta=(DisplayAfter="OnSuccessTags"))
	EChangeTagsMethod OnFailedMethod = EChangeTagsMethod::DoNothing;
	
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnSuccessMethod!=EChangeTagsMethod::DoNothing"))
	FGameplayTagContainer OnSuccessTags;
	
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnFailedMethod!=EChangeTagsMethod::DoNothing"))
	FGameplayTagContainer OnFailedTags;
};
