// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RecipeRow.generated.h"

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName  InputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Activity;
};

/**
 * 
 */
USTRUCT()
struct RECIPES_API FRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RecipeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRecipeStep> Steps;
};
