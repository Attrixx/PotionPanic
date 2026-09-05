// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "ActivityHolderTarget.h"
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
 * Edits the tags of a carried item: the one sitting on the station, the one the instigator holds,
 * or the first of the two that carries anything, depending on Target.
 */
UCLASS(DisplayName = "Change Tags")
class ACTIVITIES_API UChangeTagsActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;

	/** Whose item is retagged, and in which order the two holders are tried. */
	UPROPERTY(EditAnywhere, Category="")
	EActivityHolderTarget Target = EActivityHolderTarget::Origin;

	UPROPERTY(EditAnywhere, Category="")
	EChangeTagsMethod OnSuccessMethod = EChangeTagsMethod::DoNothing;

	UPROPERTY(EditAnywhere, Category="", meta=(DisplayAfter="OnSuccessTags"))
	EChangeTagsMethod OnFailedMethod = EChangeTagsMethod::DoNothing;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnSuccessMethod != EChangeTagsMethod::DoNothing", Categories="Item"))
	FGameplayTagContainer OnSuccessTags;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="OnFailedMethod != EChangeTagsMethod::DoNothing", Categories="Item"))
	FGameplayTagContainer OnFailedTags;
};
