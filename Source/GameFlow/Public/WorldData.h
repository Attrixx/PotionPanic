// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Rounds/Round.h"
#include "WorldData.generated.h"

class URecipeAsset;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API UWorldData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	/** Rounds making up this world, the run starting at index 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRound> Rounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int64 ScoreToSucceed;
	
	/** @return The round at Index, or nullptr if there is none (past the last round included). */
	const FRound* GetRoundAt(int32 Index) const
	{
		return Rounds.IsValidIndex(Index) ? &Rounds[Index] : nullptr;
	}
	
protected:
	
#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
