// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActivityExecutor.generated.h"

class UHolderComponent;
class AItemActor;
class UActivityAsset;
class UActivityStep;
class UActivityEvaluator;
class UActivityConclusion;
struct FActivityStepResult;

UENUM(BlueprintType)
enum class EActivityExecutionStatus : uint8
{
	NotStarted = 0,
	Ongoing,
	Success,
	Failed,
};

USTRUCT(BlueprintType)
struct FActivityExecutionState
{
	GENERATED_BODY()

	/**
	 * Holder where the Activity is executed.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UHolderComponent> Holder;

	/**
	 * Item present on the Holder. Can be null.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AItemActor> Item; // = Cast<AItemActor>(Holder->GetCarriable());

	/**
	 * Last Instigator received through StartActivity or Interact. Can be null.
	 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LastInstigator;

	UPROPERTY(BlueprintReadOnly)
	EActivityExecutionStatus Status = EActivityExecutionStatus::NotStarted;

	/**
	 * Global score of the activity.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ACTIVITIES_API UActivityExecutor : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void Initialize(UHolderComponent* HolderComponent);

	UFUNCTION(BlueprintCallable)
	void StartActivity(UActivityAsset* Activity, AActor* Instigator = nullptr);

	UFUNCTION(BlueprintCallable)
	void Interact(AActor* Instigator);

	UFUNCTION(BlueprintCallable)
	void Cancel();

	UFUNCTION(BlueprintCallable)
	EActivityExecutionStatus GetExecutionStatus() const;

private:

	UFUNCTION()
	void Holder_OnCarriableChanged(UHolderComponent* Holder);
	void ContinueExecution();
	void OnStepFinished(const FActivityStepResult& Result);
	void Reset();

private:

	UPROPERTY()
	FActivityExecutionState State;

	UPROPERTY()
	TArray<TObjectPtr<UActivityStep>> Steps;

	UPROPERTY()
	TObjectPtr<UActivityEvaluator> Evaluator;

	UPROPERTY()
	TObjectPtr<UActivityConclusion> Conclusion;

	int32 CurrentStepIndex = 0;
};
