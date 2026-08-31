// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AlchemistCustomizationAsset.generated.h"

class USkeletalMesh;

UENUM(BlueprintType)
enum class EAlchemistColor : uint8
{
	Blue,
	Orange,
	Green,
	Purple,
	Yellow,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FAlchemistColoredMesh
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FColor Color;
};

/**
 * Data asset that stores the mapping between Alchemist Color enums and their respective meshes and physical colors.
 */
UCLASS()
class PLAYER_API UAlchemistCustomizationAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TMap<EAlchemistColor, FAlchemistColoredMesh> AlchemistColors;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	USkeletalMesh* GetMesh(EAlchemistColor ColorEnum) const;

	UFUNCTION(BlueprintCallable, Category = "Customization")
	FColor GetColor(EAlchemistColor ColorEnum) const;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FLinearColor GetLinearColor(EAlchemistColor ColorEnum) const;
};
