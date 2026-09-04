// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ItemOrderQueueWidget.generated.h"

struct FItemOrder;

/**
 * 
 */
UCLASS(Abstract)
class USERINTERFACES_API UItemOrderQueueWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	
	void NativeOnInitialized() override;
	void NativeDestruct() override;
	
private:
	
	void OnOrderChanged(const FItemOrder& Order);
	
protected:
	
	UFUNCTION(BlueprintImplementableEvent)
	UWidget* CreateOrderWidget(const FItemOrder& Order);
	
	UFUNCTION(BlueprintImplementableEvent)
	void CancelOrderWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintImplementableEvent)
	void CompleteOrderWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SystemDeleteOrderWidget(UWidget* Widget);
	
private:
	
	UPROPERTY()
	TMap<uint32, UWidget*> OrderWidgetByOrderId;
};