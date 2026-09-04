// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "CreateInstigatorItemActivityConclusion.generated.h"

class AItemActor;
class UItemAsset;

/**
 * Same outcomes as "Create Item", except the new item is handed to the instigator instead of being
 * left on the station. The activity MUST guarantee the instigator is there with a free holder --
 * typically with an "Instigator Has Item" step concluded by a consume -- otherwise this asserts.
 */
UCLASS(DisplayName = "Create Instigator Item")
class ACTIVITIES_API UCreateInstigatorItemActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

public:

#if WITH_EDITOR
	void GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const override;
#endif

protected:

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;

	UPROPERTY(EditAnywhere, Category="")
	TSubclassOf<AItemActor> ItemClass;

	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bCreateOnSuccess = true;

	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bCreateOnFail = false;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bCreateOnSuccess))
	TObjectPtr<UItemAsset> OnActivitySuccess;

	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bCreateOnFail))
	TObjectPtr<UItemAsset> OnActivityFail;
};
