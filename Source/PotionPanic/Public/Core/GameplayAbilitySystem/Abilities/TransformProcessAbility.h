// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Core/GameplayAbilitySystem/Abilities/Tasks/QuickTimeEventTask.h"
#include "TransformProcessAbility.generated.h"

class APotionPanicCharacter;

UENUM(BlueprintType)
enum class EStationStepType : uint8
{
	Timer,
	QuickTimeEvent,
	WaitForGameplayEvent,
	WaitForPlayerInteraction
};

USTRUCT(BlueprintType)
struct FStationStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStationStepType StepType = EStationStepType::Timer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQuickTimeEventInput> QuickTimeEventInputs;
};

USTRUCT()
struct FQuickTimeEventSequenceTracker
{
	GENERATED_BODY()
	int32 CurrentInputIndex = 0;
	int32 SuccessfulInputs = 0;
};

UCLASS()
class POTIONPANIC_API UTransformProcessAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:

	void ExecuteNextStep();

	void ExecuteTimerStep(const FStationStep& Step);
	void ExecuteQuickTimeEvent(const FStationStep& Step);
	void ExecuteWaitForGameplayEventStep(const FStationStep& Step);
	void ExecuteWaitForPlayerInteractionStep(const FStationStep& Step);

	UFUNCTION()
	void NextQuickTimeEvent(FGameplayEventData Payload);
	void OnQuickTimeEventEnded(bool bIsSuccess);

	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnStepCompleted();

protected:

private:

	TArray<FStationStep> ProcessSteps;
	int32 CurrentStepIndex = 0;
	FStationStep CurrentStep;
	FQuickTimeEventSequenceTracker QuickTimeEventTracker;
	
};
