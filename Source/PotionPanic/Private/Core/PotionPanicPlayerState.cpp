// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicPlayerState.h"
#include "AbilitySystemComponent.h"

APotionPanicPlayerState::APotionPanicPlayerState()
{
	SetNetUpdateFrequency(100.f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}
