// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RecipeOrderWidget.generated.h"

class URecipeAsset;

/**
 * 
 */
UCLASS()
class USERINTERFACES_API URecipeOrderWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:	
	TObjectPtr<URecipeAsset> Recipe;
	
	uint32 OrderId;
	
	double BeginTime = 0.0f;
	double EndTime = 0.0f;
	double CurrentTime = 0.0f;
};
