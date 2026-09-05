// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "ActivityHolderTarget.h"
#include "MorphOrConsumeItemActivityConclusion.generated.h"

class UItemAsset;

/**
 * Morphs or destroys a carried item: the one sitting on the station, the one the instigator holds,
 * or the first of the two that carries anything, depending on Target.
 */
UCLASS(DisplayName = "Morph or Consume Item")
class ACTIVITIES_API UMorphOrConsumeItemActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()
	
public:

#if WITH_EDITOR
	void GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const override;
#endif

protected:

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;

	/** Whose item is morphed, and in which order the two holders are tried. */
	UPROPERTY(EditAnywhere, Category="")
	EActivityHolderTarget Target = EActivityHolderTarget::Origin;

	/** Toggle shown next to OnActivitySuccess. Unchecked leaves the item untouched on success. */
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnSuccess = true;
	
	/** Toggle shown next to OnActivityFail. Unchecked leaves the item untouched on failure. */
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnFail = false;
	
	/**
	 * What the item becomes on success. Leave EMPTY (with the toggle checked) to DESTROY it
	 * instead -- that is the "or Consume" half of this conclusion. Unchecking the toggle is a
	 * third, different outcome: the item is left exactly as it is.
	 */
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnSuccess))
	TObjectPtr<UItemAsset> OnActivitySuccess;
	
	/**
	 * What the item becomes on failure. Leave EMPTY (with the toggle checked) to DESTROY it.
	 * Unchecking the toggle -- the default -- leaves the item untouched on the station instead,
	 * which is rarely what a failed activity wants.
	 */
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnFail))
	TObjectPtr<UItemAsset> OnActivityFail;
};
