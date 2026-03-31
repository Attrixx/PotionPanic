// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActivityExecutionState.h"
#include "ActivityExecutor.generated.h"

class UActivityAsset;
class UActivityStep;
class UActivityEvaluator;
class UActivityConclusion;
struct FActivityStepResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActivityExecutionStatusChangedDelegate, UActivityExecutor*, Executor, EActivityExecutionStatus, NewStatus);

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class ACTIVITIES_API UActivityExecutor : public UActorComponent
{
	GENERATED_BODY()

protected:
	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	
	UPROPERTY(BlueprintAssignable)
	FActivityExecutionStatusChangedDelegate OnExecutionStatusChanged;

private:

	UFUNCTION()
	void Holder_OnCarriableChanged(UHolderComponent* Holder);
	void ContinueExecution();
	void OnStepFinished(const FActivityStepResult& Result);
	void Conclude(EActivityExecutionStatus Status); // Conclude and broadcast the status change
	void Reset(EActivityExecutionStatus Status); // Reset and broadcast the status change

	UFUNCTION()
	void OnRep_State(const FActivityExecutionState& OldState);
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_State)
	FActivityExecutionState State;

	UPROPERTY()
	TArray<TObjectPtr<UActivityStep>> Steps;

	UPROPERTY()
	TObjectPtr<UActivityEvaluator> Evaluator;

	UPROPERTY()
	TObjectPtr<UActivityConclusion> Conclusion;

	int32 CurrentStepIndex = 0;
};
