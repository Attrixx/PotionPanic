// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemOrderQueueWidget.h"
#include "AlchemyGameState.h"
#include "ItemAsset.h"
#include "Components/PanelWidget.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ItemOrderQueueWidget, Log, All);

void UItemOrderQueueWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	GameState->OnOrderChanged.AddUObject(this, &ThisClass::OnOrderChanged);
}

void UItemOrderQueueWidget::NativeDestruct()
{
	Super::NativeDestruct();

	auto* GameState = GetWorld()->GetGameState<AAlchemyGameState>();
	GameState->OnOrderChanged.RemoveAll(this);
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
