// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/InteractAbility.h"
#include "Core/PotionPanicCharacter.h"

void UInteractAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character)) return;

	Character->Interact();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
