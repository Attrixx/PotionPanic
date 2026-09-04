// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemOrderQueueWidget.h"
#include "AlchemyGameState.h"
#include "ItemAsset.h"
#include "Components/PanelWidget.h"
#include <Engine/World.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemOrderQueueWidget, Log, All);

void UItemOrderQueueWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TryBindToGameState())
		return;

	// A client builds its HUD from its player controller, which can begin play before the game
	// state has replicated in. Waiting for it beats binding to nothing: a queue that missed its
	// subscription shows no order for the whole level, and says nothing about why.
	if (UWorld* World = GetWorld())
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
}

void UItemOrderQueueWidget::NativeDestruct()
{
	Super::NativeDestruct();

	UWorld* World = GetWorld();
	if (!World)
		return;

	if (GameStateSetHandle.IsValid())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
		GameStateSetHandle.Reset();
	}

	if (auto* GameState = World->GetGameState<AAlchemyGameState>())
		GameState->OnOrderChanged.RemoveAll(this);
}

bool UItemOrderQueueWidget::TryBindToGameState()
{
	UWorld* World = GetWorld();
	auto* GameState = World ? World->GetGameState<AAlchemyGameState>() : nullptr;
	if (!GameState)
		return false;

	GameState->OnOrderChanged.AddUObject(this, &ThisClass::OnOrderChanged);

	// Orders placed before this widget existed have no broadcast left to come: replay the list as
	// it stands so the queue opens in sync instead of waiting for the next state change.
	for (const FItemOrder& Order : GameState->GetRoundOrders())
		OnOrderChanged(Order);

	return true;
}

void UItemOrderQueueWidget::OnGameStateSet(AGameStateBase* NewGameState)
{
	if (!TryBindToGameState())
		return; // Some other game state arrived: keep waiting for ours.

	GetWorld()->GameStateSetEvent.Remove(GameStateSetHandle);
	GameStateSetHandle.Reset();
}

void UItemOrderQueueWidget::OnOrderChanged(const FItemOrder& Order)
{
	switch (Order.State)
	{
	case EOrderState::Pending:
	{
		// Order is not placed yet...
	}
	break;

	case EOrderState::Placed:
	{
		if (OrderWidgetByOrderId.Contains(Order.OrderId))
			break;

		UWidget* Widget = CreateOrderWidget(Order);
		OrderWidgetByOrderId.Add(Order.OrderId, Widget);
	}
	break;

	case EOrderState::Cancelled:
	{
		UWidget* Widget;
		if (OrderWidgetByOrderId.RemoveAndCopyValue(Order.OrderId, Widget))
			CancelOrderWidget(Widget);
	}
	break;

	case EOrderState::Completed:
	{
		UWidget* Widget;
		if (OrderWidgetByOrderId.RemoveAndCopyValue(Order.OrderId, Widget))
			CompleteOrderWidget(Widget);
	}
	break;

	case EOrderState::SystemDeleted:
	{
		UWidget* Widget;
		if (OrderWidgetByOrderId.RemoveAndCopyValue(Order.OrderId, Widget))
			SystemDeleteOrderWidget(Widget);
	}
	break;

	default:
		checkNoEntry();
	}
}
