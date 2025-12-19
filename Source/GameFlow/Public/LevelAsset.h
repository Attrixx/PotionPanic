// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelAsset.generated.h"

class USectionAsset;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API ULevelAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> World;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 ScoreToSucceed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USectionAsset> StartingSection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<USectionAsset>> SectionPool;
};
