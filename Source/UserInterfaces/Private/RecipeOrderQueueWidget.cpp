// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeOrderQueueWidget.h"
#include "AlchemyGameState.h"
#include "Components/PanelWidget.h"

void URecipeOrderQueueWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	GameState->OnOrderChanged.AddUObject(this, &ThisClass::OnOrderChanged);
}

void URecipeOrderQueueWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	GameState->OnOrderChanged.RemoveAll(this);
}

void URecipeOrderQueueWidget::OnOrderChanged(const FOrder& Order)
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
		check(!OrderWidgetByOrderId.Contains(Order.OrderId));
		UWidget* Widget = CreateOrderWidget(Order);
		OrderWidgetByOrderId.Add(Order.OrderId, Widget);
	}
	break;

	case EOrderState::Cancelled:
	{
		UWidget* Widget = OrderWidgetByOrderId.FindAndRemoveChecked(Order.OrderId);
		CancelOrderWidget(Widget);
	}
	break;

	case EOrderState::Completed:
	{
		UWidget* Widget = OrderWidgetByOrderId.FindAndRemoveChecked(Order.OrderId);
		CompleteOrderWidget(Widget);
	}
	break;

	default:
		checkNoEntry();
	}
}
