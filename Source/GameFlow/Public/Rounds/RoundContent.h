// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoundContent.generated.h"

class UStationsLayoutLayer;
class URecipeAsset;

USTRUCT(BlueprintType)
struct GAMEFLOW_API FRoundContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UStationsLayoutLayer>> NewLayers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<URecipeAsset>> NewRecipes;
};
