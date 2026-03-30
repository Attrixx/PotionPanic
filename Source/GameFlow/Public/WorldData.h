// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldData.generated.h"

class URecipeAsset;
class USectionAsset;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API UWorldData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	// TODO: Change this array to the choices on each round
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<URecipeAsset>> Recipes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 ScoreToSucceed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USectionAsset> StartingSection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<USectionAsset>> SectionPool;
};
