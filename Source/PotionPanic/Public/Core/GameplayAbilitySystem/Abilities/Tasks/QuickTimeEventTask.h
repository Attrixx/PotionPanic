// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "QuickTimeEventTask.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FQuickTimeEventEnded, bool /* bSuccess */);

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

	void OnTimeWindowExpired();

private:

	FQuickTimeEventInput InputData;

	FDelegateHandle GameplayEventHandle;
	FTimerHandle WindowTimerHandle;
	
};
