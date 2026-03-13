// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RecipeAsset.generated.h"

class UActivityAsset;

/**
 * 
 */
UCLASS()
class RECIPES_API URecipeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RecipeName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UActivityAsset>> Steps;
};
