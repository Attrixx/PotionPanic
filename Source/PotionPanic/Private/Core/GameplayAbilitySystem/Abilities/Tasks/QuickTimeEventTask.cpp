// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/Abilities/Tasks/QuickTimeEventTask.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"
#include "HAL/PlatformTime.h"

UQuickTimeEventTask* UQuickTimeEventTask::QuickTimeEvent(UGameplayAbility* OwningAbility, const FQuickTimeEventInput& Input)
{
	UQuickTimeEventTask* MyObj = NewAbilityTask<UQuickTimeEventTask>(OwningAbility);
	UE_LOG(LogTemp, Warning, TEXT("Creating Task for %s (%.6f)"), *Input.KeyTag.ToString(), FPlatformTime::Seconds());
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

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	FGameplayTagContainer KeyTagContainer = TagsManager.RequestGameplayTagChildren(FGameplayTag::RequestGameplayTag("Keys"));

	for (const FGameplayTag& KeyTag : KeyTagContainer)
	{
		FDelegateHandle Handle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(KeyTag).AddUObject(
			this,
			&UQuickTimeEventTask::OnGameplayEventReceived
		);
		GameplayEventHandles.Add(TPair<FGameplayTag, FDelegateHandle>(KeyTag, Handle));
	}

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
	UnregisterGameplayEvents();
}

void UQuickTimeEventTask::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	const float RemainingTime = Ability->GetWorld()->GetTimerManager().GetTimerRemaining(WindowTimerHandle);
	Ability->GetWorld()->GetTimerManager().ClearTimer(WindowTimerHandle);
	UnregisterGameplayEvents();

	if (bIsFinished) return;
	bIsFinished = true;
	
	if (InputData.KeyTag != Payload->EventTag)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("QTE Fail: Wrong Key %s"), *InputData.KeyTag.ToString()));
		OnQuickTimeEventEnded.Broadcast(false, RemainingTime);
		EndTask();
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, FString::Printf(TEXT("QTE Success: %s"), *InputData.KeyTag.ToString()));
	OnQuickTimeEventEnded.Broadcast(true, RemainingTime);
	EndTask();
}

void UQuickTimeEventTask::UnregisterGameplayEvents()
{
	if (AbilitySystemComponent.IsValid())
	{
		for (const TPair<FGameplayTag, FDelegateHandle>& Pair : GameplayEventHandles)
		{
			AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(Pair.Key).Remove(Pair.Value);
		}
		GameplayEventHandles.Empty();
	}
}

void UQuickTimeEventTask::OnTimeWindowExpired()
{
	UnregisterGameplayEvents();
	if (bIsFinished) return;
	bIsFinished = true;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("QTE Fail: %s"), *InputData.KeyTag.ToString()));
	OnQuickTimeEventEnded.Broadcast(false, 0.f);
	EndTask();
}
