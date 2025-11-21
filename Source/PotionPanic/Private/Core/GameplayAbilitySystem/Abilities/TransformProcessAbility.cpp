// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/TransformProcessAbility.h"
#include "Core/StationActor.h"
#include "Core/PotionPanicCharacter.h"
#include "Core/GameplayAbilitySystem/Abilities/Tasks/QuickTimeEventTask.h"
#include "RecipeSystem/Recipe.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

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
		AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
		if (IsValid(Station))
		{
			Station->ShowInteractionUI(false);
			Station->HideProgressUI();
		}
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const FStationStep& Step = ProcessSteps[CurrentStepIndex];
	switch (Step.StepType)
	{
	case EStationStepType::Timer:
		ExecuteTimerStep(Step);
		break;
	case EStationStepType::QuickTimeEvent:
		ExecuteQuickTimeEvent(Step);
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
	AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
	if (IsValid(Station))
	{
		Station->ShowAnimatedProgress(Step.Duration, false);
	}

	auto* Task = UAbilityTask_WaitDelay::WaitDelay(this, Step.Duration);
	if (Task)
	{
		Task->OnFinish.AddDynamic(this, &UTransformProcessAbility::OnStepCompleted);
		Task->ReadyForActivation();
	}
}

void UTransformProcessAbility::ExecuteQuickTimeEvent(const FStationStep& Step)
{
	CurrentStep = Step;

	AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
	if (IsValid(Station))
	{
		Station->HideProgressUI();
		Station->ShowInteractionUI(true);
	}

	// Wait for interaction
	auto* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Step.EventTag);
	if (Task)
	{
		Task->EventReceived.AddDynamic(this, &UTransformProcessAbility::NextQuickTimeEvent);
		Task->ReadyForActivation();
	}

	QuickTimeEventTracker = FQuickTimeEventSequenceTracker();
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

void UTransformProcessAbility::NextQuickTimeEvent(FGameplayEventData Payload)
{
	AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
	if (!IsValid(Station)) return;
	APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(Station->GetCurrentProcessInstigator());
	if (!IsValid(Character)) return;
	if (QuickTimeEventTracker.CurrentInputIndex == 0)
	{
		Station->ShowInteractionUI(false);
		Character->OnStartUsingStation();
	}

	Character->ShowQuickTimeEventInputKey(CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex].KeyTag);

	auto* Task = UQuickTimeEventTask::QuickTimeEvent(this, CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex]);
	if (Task)
	{
		Task->OnQuickTimeEventEnded.AddUObject(this, &UTransformProcessAbility::OnQuickTimeEventEnded);
		Task->ReadyForActivation();
	}
}

void UTransformProcessAbility::OnQuickTimeEventEnded(bool bIsSuccess)
{
	if (bIsSuccess)
	{
		QuickTimeEventTracker.SuccessfulInputs++;
	}
	QuickTimeEventTracker.CurrentInputIndex++;
	if (QuickTimeEventTracker.CurrentInputIndex < CurrentStep.QuickTimeEventInputs.Num())
	{
		NextQuickTimeEvent(FGameplayEventData());
	}
	else
	{
		if (QuickTimeEventTracker.SuccessfulInputs * 1.f < CurrentStep.QuickTimeEventInputs.Num() * 0.5f)
		{
			CurrentStepIndex--;
		}

		AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
		if (IsValid(Station))
		{
			APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(Station->GetCurrentProcessInstigator());
			if (IsValid(Character))
			{
				Character->HideQuickTimeEventWidget();
			}
		}

		OnStepCompleted();
	}
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
