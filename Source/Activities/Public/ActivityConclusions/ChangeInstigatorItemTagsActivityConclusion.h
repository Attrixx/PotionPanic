// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "ActivityConclusions/ChangeTagsActivityConclusion.h"
#include "GameplayTagContainer.h"
#include "ChangeInstigatorItemTagsActivityConclusion.generated.h"

/**
 * Same outcomes as "Change Tags", applied to the item the instigator carries instead of the one
 * sitting on the station. The activity MUST guarantee the instigator is there and holds an item --
 * typically with an "Instigator Has Item" step -- otherwise this asserts.
 */
UCLASS(DisplayName = "Change Instigator Item Tags")
class ACTIVITIES_API UChangeInstigatorItemTagsActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;

	UPROPERTY(EditAnywhere, Category="")
	EChangeTagsMethod OnSuccessMethod = EChangeTagsMethod::DoNothing;

	UPROPERTY(EditAnywhere, Category="", meta=(DisplayAfter="OnSuccessTags"))
	EChangeTagsMethod OnFailedMethod = EChangeTagsMethod::DoNothing;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnSuccessMethod != EChangeTagsMethod::DoNothing", Categories="Item"))
	FGameplayTagContainer OnSuccessTags;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnFailedMethod != EChangeTagsMethod::DoNothing", Categories="Item"))
	FGameplayTagContainer OnFailedTags;
};
