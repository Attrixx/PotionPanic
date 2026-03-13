// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "CreateItemActivityConclusion.generated.h"

class UItemAsset;

/**
 * 
 */
UCLASS(DisplayName = "Create Item")
class ACTIVITIES_API UCreateItemActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
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
