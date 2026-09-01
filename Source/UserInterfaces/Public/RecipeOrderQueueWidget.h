// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RecipeOrderQueueWidget.generated.h"

struct FOrder;

/**
 * 
 */
UCLASS(Abstract)
class USERINTERFACES_API URecipeOrderQueueWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	
	void NativeOnInitialized() override;
	void NativeDestruct() override;
	
private:
	
	void OnOrderChanged(const FOrder& Order);
	
protected:
	
	UFUNCTION(BlueprintImplementableEvent)
	UWidget* CreateOrderWidget(const FOrder& Order);
	
	UFUNCTION(BlueprintImplementableEvent)
	void CancelOrderWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintImplementableEvent)
	void CompleteOrderWidget(UWidget* Widget);
	
private:
	
	UPROPERTY()
	TMap<uint32, UWidget*> OrderWidgetByOrderId;
};