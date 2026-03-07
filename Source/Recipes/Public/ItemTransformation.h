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
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TransformationName;
	
	// Item tags and Activity tags needed on the Item and Station to perform this transformation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item,Activity"))
	FGameplayTagContainer InputTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced);
	TArray<TObjectPtr<UActivityStepSettings>> ActivitySteps;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemAsset> OutputItem;
};
