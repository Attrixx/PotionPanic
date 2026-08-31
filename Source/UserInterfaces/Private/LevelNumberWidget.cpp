// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelNumberWidget.h"

#include "Components/Image.h"

void ULevelNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULevelNumberWidget::SetLevelNumber_Implementation(int32 LevelNumber)
{
	if (LevelNumberImage)
	{
		UMaterialInstanceDynamic* DynamicMaterial = LevelNumberImage->GetDynamicMaterial();
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(TEXT("Number"), LevelNumber);
		}
	}
}

void ULevelNumberWidget::OnLevelUnlocked_Implementation()
{
	if (PrintNumber)
	{
		PlayAnimation(PrintNumber);
	}
}
