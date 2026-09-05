// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "ActivityHolderTarget.h"
#include "CreateItemActivityConclusion.generated.h"

class AItemActor;
class UItemAsset;

/**
 * Spawns an item and hands it to a holder: the station's, the instigator's, or the first of the two
 * that is free, depending on Target. When every candidate holder is full the item is still spawned,
 * but dropped at the preferred holder instead of being handed over.
 */
UCLASS(DisplayName = "Create Item")
class ACTIVITIES_API UCreateItemActivityConclusion : public UActivityConclusion
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

	/** Which holder receives the new item, and in which order the two are tried. */
	UPROPERTY(EditAnywhere, Category="")
	EActivityHolderTarget Target = EActivityHolderTarget::Origin;

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
