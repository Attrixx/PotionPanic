// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicCharacter.h"

APotionPanicCharacter::APotionPanicCharacter()
{
}

void APotionPanicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APotionPanicCharacter::OnInteract()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("ON INTERACT"));
	}
}

void APotionPanicCharacter::OnCarry()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("ON CARRY"));
	}
}
