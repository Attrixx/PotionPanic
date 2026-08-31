// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemistCustomizationAsset.h"

USkeletalMesh* UAlchemistCustomizationAsset::GetMesh(EAlchemistColor ColorEnum) const
{
	if (const FAlchemistColoredMesh* ColoredMesh = AlchemistColors.Find(ColorEnum))
	{
		return ColoredMesh->Mesh;
	}
	return nullptr;
}

FColor UAlchemistCustomizationAsset::GetColor(EAlchemistColor ColorEnum) const
{
	if (const FAlchemistColoredMesh* ColoredMesh = AlchemistColors.Find(ColorEnum))
	{
		return ColoredMesh->Color;
	}
	return FColor::White;
}

FLinearColor UAlchemistCustomizationAsset::GetLinearColor(EAlchemistColor ColorEnum) const
{
	return FLinearColor(GetColor(ColorEnum));
}
