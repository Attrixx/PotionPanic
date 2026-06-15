// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldData.generated.h"

class URecipeAsset;
class URoundTree;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API UWorldData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<URoundTree> Rounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 ScoreToSucceed;
	
protected:
	
#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
