// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemTransformation.generated.h"

class UItemAsset;
class UActivityAsset;
class UIteractionSetting;

UCLASS()
class RECIPES_API UItemTransformation : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText StepName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemAsset> InputItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UActivityAsset> Activity;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UIteractionSetting>> InteractionSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemAsset> OutputItem;
};
