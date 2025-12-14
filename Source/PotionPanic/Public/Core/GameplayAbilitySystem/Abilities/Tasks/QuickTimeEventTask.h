// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "QuickTimeEventTask.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FQuickTimeEventEnded, bool /* bSuccess */, float /* RemainingTime */);

USTRUCT(BlueprintType)
struct FQuickTimeEventInput
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag KeyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputTimeWindow = 1.f;
};

UCLASS()
class POTIONPANIC_API UQuickTimeEventTask : public UAbilityTask
{
	GENERATED_BODY()

public:

	static UQuickTimeEventTask* QuickTimeEvent(UGameplayAbility* OwningAbility, const FQuickTimeEventInput& Input);

	FQuickTimeEventEnded OnQuickTimeEventEnded;

protected:

	void Activate() override;
	void OnDestroy(bool bInOwnerFinished) override;

private:

	void OnGameplayEventReceived(const FGameplayEventData* Payload);
	void UnregisterGameplayEvents();

	void OnTimeWindowExpired();

private:

	FQuickTimeEventInput InputData;

	TArray<TPair<FGameplayTag, FDelegateHandle>> GameplayEventHandles;
	FTimerHandle WindowTimerHandle;

	bool bIsFinished = false;
	
};
