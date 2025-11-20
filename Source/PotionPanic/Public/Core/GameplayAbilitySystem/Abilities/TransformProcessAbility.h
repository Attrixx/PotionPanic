// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TransformProcessAbility.generated.h"

UENUM(BlueprintType)
enum class EStationStepType : uint8
{
	Timer,
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
};

//USTRUCT()
//struct FTransformProcessAbilitySpec : public FGameplayAbilitySpec
//{
//	GENERATED_BODY()
//
//	FRecipe Recipe;
//
//	FTransformProcessAbilitySpec() {};
//	FTransformProcessAbilitySpec(TSubclassOf<UGameplayAbility> InAbility, int32 Level)
//		: FGameplayAbilitySpec(InAbility, Level)
//	{};
//};

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
	void ExecuteWaitForGameplayEventStep(const FStationStep& Step);
	void ExecuteWaitForPlayerInteractionStep(const FStationStep& Step);

	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnStepCompleted();

protected:

private:

	TArray<FStationStep> ProcessSteps;
	int32 CurrentStepIndex = 0;
	
};
