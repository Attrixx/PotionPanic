// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ActivityStep.generated.h"

class UActivityExecutor;
struct FActivityStepResult;

DECLARE_DELEGATE_OneParam(FActivityStepResultDelegate, const FActivityStepResult &);

/**
 * Abstract base class representing a single modular step within an Activity.
 * Designed to be subclassed in C++ or Blueprint to implement custom step logic.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityStep : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Starts the step
	 * @param LastInstigator The actor which last interacted with the activity this step is part of.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void StartStep(AActor* LastInstigator);

	/**
	 *
	 * @param Instigator Actor triggering the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnInteract(AActor* Instigator);

	/**
	 * Cancels the step.
	 * Implementation should NOT call FinishStep after being canceled.
	 * Make sure to correctly clean up any timer or bindings here.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void CancelStep();

	/**
	 * Routes one of the four abstract inputs to the step, sent by an actor whose inputs it captured
	 * through IActivityInputCapture. Steps that never capture anything can ignore it.
	 * @param Instigator Actor the press came from. Implementations must check it is the one they
	 *        captured: nothing stops another player's pawn from reaching here.
	 * @param Slot The input that was pressed.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnActivityInput(AActor* Instigator, EActivityInputSlot Slot);

protected:

	/**
	 * Notify the step executor that this step is finished.
	 * @param Output Result to report.
	 */
	UFUNCTION(BlueprintPure = false)
	void FinishStep(const FActivityStepResult& Output) const;

	/** @return The executor running this step. Never null in practice: it is this step's outer. */
	UFUNCTION(BlueprintPure)
	UActivityExecutor* GetExecutor() const;

private:

	friend class UActivityExecutor;
	FActivityStepResultDelegate StepFinishedCallback;

protected: // Default implementations
	virtual void StartStep_Implementation(AActor* LastInstigator);
	virtual void OnInteract_Implementation(AActor* Instigator);
	virtual void CancelStep_Implementation();
	virtual void OnActivityInput_Implementation(AActor* Instigator, EActivityInputSlot Slot);
	virtual void OnCancelRequested_Implementation(AActor* Instigator);
};
