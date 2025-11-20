// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/PickupAbility.h"
#include "Core/PotionPanicCharacter.h"

void UPickupAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character)) return;

	Character->PickupObject();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
