// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/DashAbility.h"
#include "Core/PotionPanicCharacter.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

void UDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character)) return;

	UAbilityTask_ApplyRootMotionConstantForce* DashTask =
		UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this,
			NAME_None,
			Character->GetActorForwardVector(),
			1300.f,
			0.15f,
			false,
			nullptr,
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
			FVector::ZeroVector,
			0.f,
			false
		);

	if (DashTask)
	{
		DashTask->ReadyForActivation();
		Character->OnDash();
		CommitAbility(Handle, ActorInfo, ActivationInfo);

		DashTask->OnFinish.AddDynamic(this, &UDashAbility::OnFinishTask);
	}
}

void UDashAbility::OnFinishTask()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
