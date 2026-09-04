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

	/**
	 * Initializes the executor with a holder component.
	 * If the executor was already bound to another Holder, the previous binding is cleared.
	 * @param HolderComponent The component responsible for holding the interactive object. Must be valid.
	 */
	UFUNCTION(BlueprintCallable)
	void Initialize(UHolderComponent* HolderComponent);

	/**
	 * Starts the execution of a new activity from a definition asset.
     * Automatically cancel any currently ongoing activity.
	 * @param Activity Activity The asset defining the activity structure (Steps, Evaluator, Conclusion).
	 * @param Instigator The optional actor that initiated the activity.
	 */
	UFUNCTION(BlueprintCallable)
	void StartActivity(UActivityAsset* Activity, AActor* Instigator = nullptr);

	/**
	 * Forwards an interaction event to the currently executing activity step.
	 * Does nothing if there is no ongoing activity.
	 * The current step is responsible for handling the internal logic triggered by this interaction.
	 * @param Instigator The actor triggering the interaction.
	 */
	UFUNCTION(BlueprintCallable)
	void Interact(AActor* Instigator);

	/**
	 * Interrupts the ongoing activity and resets the executor's state.
	 * Propagates the cancellation to the current step to allow it to execute its cleanup logic.
	 */
	UFUNCTION(BlueprintCallable)
	void Cancel();

	UFUNCTION(BlueprintCallable)
	EActivityExecutionStatus GetExecutionStatus() const;

	/** Context of the last started activity: holder, item, instigator, status and score. */
	const FActivityExecutionState& GetExecutionState() const { return State; }

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

	// Guards against calling CancelStep before StartStep.
	// Must be reset when writing to CurrentStepIndex.
	bool bCurrentStepStarted = false;
};
