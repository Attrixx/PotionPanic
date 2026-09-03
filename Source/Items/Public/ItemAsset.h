// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemAsset.generated.h"

class UStaticMesh;
class UNiagaraSystem;
class USoundBase;

/**
 * 
 */
UCLASS()
class ITEMS_API UItemAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	
#if WITH_EDITOR
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	FTransform StaticMeshTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<USoundBase> LoopSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<USoundBase> BreakSound = nullptr;
};
