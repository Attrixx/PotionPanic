// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/Tasks/QuickTimeEventTask.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

UQuickTimeEventTask* UQuickTimeEventTask::QuickTimeEvent(UGameplayAbility* OwningAbility, const FQuickTimeEventInput& Input)
{
	UQuickTimeEventTask* MyObj = NewAbilityTask<UQuickTimeEventTask>(OwningAbility);
	MyObj->InputData = Input;
	return MyObj;
}

void UQuickTimeEventTask::Activate()
{
	if (!Ability || !AbilitySystemComponent.IsValid())
	{
		EndTask();
		return;
	}

	GameplayEventHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(InputData.KeyTag).AddUObject(
		this,
		&UQuickTimeEventTask::OnGameplayEventReceived
	);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Press %s within %.2f seconds"), *InputData.KeyTag.ToString().ToUpper(), InputData.InputTimeWindow));

	Ability->GetWorld()->GetTimerManager().SetTimer(
		WindowTimerHandle,
		this,
		&UQuickTimeEventTask::OnTimeWindowExpired,
		InputData.InputTimeWindow,
		false
	);
}

void UQuickTimeEventTask::OnDestroy(bool bInOwnerFinished)
{
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(InputData.KeyTag).Remove(GameplayEventHandle);
	}
}

void UQuickTimeEventTask::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	Ability->GetWorld()->GetTimerManager().ClearTimer(WindowTimerHandle);
	OnQuickTimeEventEnded.Broadcast(true);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("QTE Success"), *InputData.KeyTag.ToString()));
	EndTask();
}

void UQuickTimeEventTask::OnTimeWindowExpired()
{
	OnQuickTimeEventEnded.Broadcast(false);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("QTE Fail"), *InputData.KeyTag.ToString()));
	EndTask();
}
