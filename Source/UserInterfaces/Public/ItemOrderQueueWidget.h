// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ItemOrderQueueWidget.generated.h"

struct FItemOrder;
class AGameStateBase;

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

	/**
	 * Subscribes to the game state and replays the orders it already holds.
	 * @return False when the world has no AAlchemyGameState yet, nothing having been bound.
	 */
	bool TryBindToGameState();

	/** Retries the binding when the world receives the game state this widget was waiting for. */
	void OnGameStateSet(AGameStateBase* NewGameState);

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
	TMap<int32, UWidget*> OrderWidgetByOrderId;

	/** Valid only while waiting for the game state, so the wait can be dropped once it is over. */
	FDelegateHandle GameStateSetHandle;
};