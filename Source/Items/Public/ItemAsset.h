// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemAsset.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UNiagaraSystem;
class USoundBase;

/**
 * Item visual/metadata definition.
 * Contains data only: no recipe/station/game-mode logic.
 */
UCLASS()
class ITEMS_API UItemAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UMaterialInterface> MaterialOverride = nullptr;

	/** Persistent visual effect attached to the item actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	/** Persistent looping sound attached to the item actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USoundBase> Sound = nullptr;

	/** One-shot effect played when the item is spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Lifecycle")
	TObjectPtr<UNiagaraSystem> SpawnEffect = nullptr;

	/** One-shot effect played when the item is destroyed/broken. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Lifecycle")
	TObjectPtr<UNiagaraSystem> DestroyEffect = nullptr;

	/** One-shot sound played when the item is spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Lifecycle")
	TObjectPtr<USoundBase> SpawnSound = nullptr;

	/** One-shot sound played when the item is destroyed/broken. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|Lifecycle")
	TObjectPtr<USoundBase> DestroySound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	bool bIsDestructible = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (EditCondition = "bIsDestructible", ClampMin = "0.0"))
	float BreakImpulseThreshold = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	bool bIsContainer = false;

	/** Data-only state flags describing possible transformations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transformation")
	TArray<FName> TransformationFlags;

	/** Additional designer tags for filtering/grouping item variants. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transformation")
	TArray<FName> DataTags;

	// TODO (Nath): Migrate TransformationFlags/DataTags to GameplayTags if/when tag dependency is enabled project-wide.
};
