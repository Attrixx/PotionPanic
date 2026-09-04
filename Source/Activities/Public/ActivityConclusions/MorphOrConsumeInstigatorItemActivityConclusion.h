// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "MorphOrConsumeInstigatorItemActivityConclusion.generated.h"

class UItemAsset;

/**
 * Same outcomes as "Morph or Consume Item", applied to the item the instigator carries instead of
 * the one sitting on the station. The activity MUST guarantee the instigator is there and holds an
 * item -- typically with an "Instigator Has Item" step -- otherwise this asserts.
 */
UCLASS(DisplayName = "Morph or Consume Instigator Item")
class ACTIVITIES_API UMorphOrConsumeInstigatorItemActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	void GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const override;
#endif

protected:

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;

	/** Toggle shown next to OnActivitySuccess. Unchecked leaves the item untouched on success. */
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnSuccess = true;

	/** Toggle shown next to OnActivityFail. Unchecked leaves the item untouched on failure. */
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnFail = false;

	/**
	 * What the carried item becomes on success. Leave EMPTY (with the toggle checked) to DESTROY it
	 * instead -- that is the "or Consume" half of this conclusion. Unchecking the toggle is a
	 * third, different outcome: the item is left exactly as it is.
	 */
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnSuccess))
	TObjectPtr<UItemAsset> OnActivitySuccess;

	/**
	 * What the carried item becomes on failure. Leave EMPTY (with the toggle checked) to DESTROY it.
	 * Unchecking the toggle -- the default -- leaves the instigator holding it untouched.
	 */
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnFail))
	TObjectPtr<UItemAsset> OnActivityFail;
};
