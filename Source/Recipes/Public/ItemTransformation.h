// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemTransformation.generated.h"

class UActivityStepSettings;
class UItemAsset;

UCLASS()
class RECIPES_API UItemTransformation : public UPrimaryDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TransformationName;
	
	// Tags that must be present on the item or the station's implemented activities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item,Activity"))
	FGameplayTagContainer InputTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced);
	TArray<TObjectPtr<UActivityStepSettings>> ActivitySteps;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemAsset> OutputItem;
};
