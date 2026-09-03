// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "MorphOrConsumeItemActivityConclusion.generated.h"

class UItemAsset;

/**
 * 
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
	
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnSuccess = true;
	
	UPROPERTY(EditAnywhere, Category="", meta=(InlineEditConditionToggle))
	bool bMorphOnFail = false;
	
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnSuccess))
	TObjectPtr<UItemAsset> OnActivitySuccess;
	
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition=bMorphOnFail))
	TObjectPtr<UItemAsset> OnActivityFail;
};
