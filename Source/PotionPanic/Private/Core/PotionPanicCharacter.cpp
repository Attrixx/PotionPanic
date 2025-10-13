// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicCharacter.h"
#include "Core/CamTargetComponent.h"

APotionPanicCharacter::APotionPanicCharacter()
{
	CamTargetComponent = CreateDefaultSubobject<UCamTargetComponent>(TEXT("CamTargetComponent"));
}

void APotionPanicCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APotionPanicCharacter::OnInteract()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("ON INTERACT"));
}

void APotionPanicCharacter::OnCarry()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("ON CARRY"));
}
