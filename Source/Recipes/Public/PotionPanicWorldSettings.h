// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "RecipeAsset.h"
#include "PotionPanicWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class RECIPES_API APotionPanicWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="PotionPanic")
	TObjectPtr<URecipeAsset> RecipeAsset;
};
