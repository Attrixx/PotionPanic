// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "RecipeAsset.h"
#include "AlchemyWorldSettings.generated.h"

/**
 * TODO: Move into CoreGameplay module
 */
UCLASS()
class GAMEFLOW_API AAlchemyWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="PotionPanic")
	TObjectPtr<URecipeAsset> RecipeAsset;
};
