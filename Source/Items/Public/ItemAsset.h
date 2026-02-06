// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	bool bIsDestructible = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	bool bIsContainer = false;
};
