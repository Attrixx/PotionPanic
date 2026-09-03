// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityConclusion.h"
#include "CompositeActivityConclusion.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Composite")
class ACTIVITIES_API UCompositeActivityConclusion : public UActivityConclusion
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	void GatherItemsProducedOnSuccess(TSet<const UItemAsset*>& OutItems) const override;
#endif

protected:

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	void Conclude_Implementation(const FActivityExecutionState& ActivityState) const override;
	
	UPROPERTY(EditAnywhere, Instanced, Category="")
	TArray<TObjectPtr<UActivityConclusion>> Conclusions;
};
