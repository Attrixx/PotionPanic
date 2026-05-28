// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoundNode.h"
#include "RoundTree.generated.h"

/**
 * 
 */
UCLASS()
class GAMEFLOW_API URoundTree : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	const FRoundNode& GetRoot() const { return Nodes[0]; }
	const FRoundNode* GetNodeAt(int32 Index) const { return Nodes.IsValidIndex(Index) ? &Nodes[Index] : nullptr; }

protected:
	
	void PostInitProperties() override;
	void PostLoad() override;

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif
	
	void EnsureRootExists();

	// Should always contain at least one element, the root
	UPROPERTY(EditAnywhere, Category = "")
	TArray<FRoundNode> Nodes;
};
