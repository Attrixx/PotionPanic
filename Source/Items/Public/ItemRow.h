// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemRow.generated.h"

class UStaticMesh;
class UNiagaraSystem;
class USoundBase;

/**
 * 
 */
USTRUCT()
struct ITEMS_API FItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsBreakable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound = nullptr;
};
