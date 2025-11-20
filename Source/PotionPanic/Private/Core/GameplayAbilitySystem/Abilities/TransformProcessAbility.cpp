// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/TransformProcessAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/StationActor.h"
#include "RecipeSystem/Recipe.h"

void UTransformProcessAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AStationActor* Station = Cast<AStationActor>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Station)) return;

	ProcessSteps = Station->GetCurrentRecipe()->Steps;

	CurrentStepIndex = 0;
	ExecuteNextStep();
}

void UTransformProcessAbility::ExecuteNextStep()
{
	if (CurrentStepIndex >= ProcessSteps.Num())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const FStationStep& Step = ProcessSteps[CurrentStepIndex];
	switch (Step.StepType)
	{
	case EStationStepType::Timer:
		ExecuteTimerStep(Step);
		break;
	case EStationStepType::WaitForGameplayEvent:
		ExecuteWaitForGameplayEventStep(Step);
		break;
	case EStationStepType::WaitForPlayerInteraction:
		ExecuteWaitForPlayerInteractionStep(Step);
		break;
	}
}

void UTransformProcessAbility::ExecuteTimerStep(const FStationStep& Step)
{
	auto* Task = UAbilityTask_WaitDelay::WaitDelay(this, Step.Duration);
	if (Task)
	{
		Task->OnFinish.AddDynamic(this, &UTransformProcessAbility::OnStepCompleted);
		Task->ReadyForActivation();
	}
}

void UTransformProcessAbility::ExecuteWaitForGameplayEventStep(const FStationStep& Step)
{
	auto* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Step.EventTag);
	if (Task)
	{
		Task->EventReceived.AddDynamic(this, &UTransformProcessAbility::OnEventReceived);
		Task->ReadyForActivation();
	}
}

void UTransformProcessAbility::ExecuteWaitForPlayerInteractionStep(const FStationStep& Step)
{
}

void UTransformProcessAbility::OnEventReceived(FGameplayEventData Payload)
{
	OnStepCompleted();
}

void UTransformProcessAbility::OnStepCompleted()
{
	CurrentStepIndex++;
	ExecuteNextStep();
}
