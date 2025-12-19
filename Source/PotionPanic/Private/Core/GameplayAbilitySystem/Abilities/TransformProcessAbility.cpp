// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameplayAbilitySystem/Abilities/TransformProcessAbility.h"
#include "Core/StationActor.h"
#include "Core/PotionPanicCharacter.h"
#include "Core/GameplayAbilitySystem/Abilities/Tasks/QuickTimeEventTask.h"
#include "RecipeSystem/Recipe.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "GameFramework/Pawn.h"

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
			Station->Multicast_ShowInteractionUI(false);
			Station->Multicast_HideProgressUI();
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
		Station->Multicast_ShowAnimatedProgress(Step.Duration, false);
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
		Station->Multicast_HideProgressUI();
		Station->Multicast_ShowInteractionUI(true);
	}

	// Wait for interaction
	auto* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Step.EventTag, nullptr, true);
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

	if (APawn* QTEInstigator = Cast<APawn>(const_cast<AActor*>(Payload.Instigator.Get())))
	{
		Station->SetCurrentProcessInstigator(QTEInstigator);
	}

	APotionPanicCharacter* Character = Cast<APotionPanicCharacter>(Station->GetCurrentProcessInstigator());
	if (!IsValid(Character)) return;
	if (QuickTimeEventTracker.CurrentInputIndex == 0)
	{
		Station->Multicast_ShowInteractionUI(false);
		Character->OnStartUsingStation();
	}

	Character->Client_ShowQuickTimeEventWidget(CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex].KeyTag, CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex].InputTimeWindow);

	auto* Task = UQuickTimeEventTask::QuickTimeEvent(this, CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex]);
	if (Task)
	{
		Task->OnQuickTimeEventEnded.AddUObject(this, &UTransformProcessAbility::OnQuickTimeEventEnded);
		Task->ReadyForActivation();
	}
}

void UTransformProcessAbility::OnQuickTimeEventEnded(bool bIsSuccess, float RemainingTime)
{
	if (bIsSuccess)
	{
		QuickTimeEventTracker.SuccessfulInputs++;
		const float PerfectTimingWindow = CurrentStep.QuickTimeEventInputs[QuickTimeEventTracker.CurrentInputIndex].InputTimeWindow * 0.25f;
		if (RemainingTime <= PerfectTimingWindow) QuickTimeEventTracker.CumulativePrecision += 1.f;
		else
		{
			QuickTimeEventTracker.CumulativePrecision += 1.f - (RemainingTime - PerfectTimingWindow) / PerfectTimingWindow;
		}
	}

	AStationActor* Station = Cast<AStationActor>(CurrentActorInfo->AvatarActor.Get());
	APotionPanicCharacter* Character = nullptr;
	if (IsValid(Station))
	{
		Character = Cast<APotionPanicCharacter>(Station->GetCurrentProcessInstigator());
		if (IsValid(Character))
		{
			Character->Client_OnQuickTimeEventStepEnd(bIsSuccess);
		}
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

		const float AveragePrecision = QuickTimeEventTracker.CumulativePrecision / CurrentStep.QuickTimeEventInputs.Num();
		// TODO What to do with AveragePrecision?

		if (IsValid(Character))
		{
			Character->OnStopUsingStation();
			Character->Client_HideQuickTimeEventWidget();
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
