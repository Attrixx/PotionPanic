// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeOrderQueueWidget.h"
#include "RecipeOrderWidget.h"
#include "Components/PanelWidget.h"

uint32 URecipeOrderQueueWidget::AddOrder(URecipeAsset* Recipe, double TimeToComplete)
{	
	check(Recipe && TimeToComplete > 0.0);
	
	auto* Child = CreateWidget<URecipeOrderWidget>(this);
	check(Child);
	
	Child->BeginTime = GetWorld()->GetTimeSeconds();
	Child->CurrentTime = Child->BeginTime;
	Child->EndTime = Child->BeginTime + TimeToComplete;
	
	Children.Add(Child);
	OrdersContainer->AddChild(Child);
	
	++OrderIdCounter;
	Child->OrderId = OrderIdCounter;
	
	return OrderIdCounter;
}

void URecipeOrderQueueWidget::RemoveOrder(uint32 InOrderId)
{
	int32 RemovedCount = Children.RemoveAllSwap([InOrderId](URecipeOrderWidget* Child)
	{
		return Child->OrderId == InOrderId;
	});
	
	check(RemovedCount == 1);
}

void URecipeOrderQueueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	double CurrentTime = GetWorld()->GetTimeSeconds();
	
	for (auto* Child : Children)
	{
		Child->CurrentTime = CurrentTime;
	}
}
