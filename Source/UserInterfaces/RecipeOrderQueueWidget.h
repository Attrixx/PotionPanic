// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RecipeOrderQueueWidget.generated.h"

class UPanelWidget;
class URecipeAsset;
class URecipeOrderWidget;

/**
 * 
 */
UCLASS()
class USERINTERFACES_API URecipeOrderQueueWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	uint32 AddOrder(URecipeAsset* Recipe, double TimeToComplete);
	void RemoveOrder(uint32 InOrderId);
	
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	TArray<URecipeOrderWidget*> Children;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> OrdersContainer;
	
	uint32 OrderIdCounter = 0;
};
