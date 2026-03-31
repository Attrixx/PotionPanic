// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ActivityStep.generated.h"

UENUM(BlueprintType)
enum class EActivityStepStatus : uint8
{
	// CriticalSuccess, // TODO Maybe ?
	Success,
	Fail,
	CriticalFail
};

USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityStepResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EActivityStepStatus Status = EActivityStepStatus::Success;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;
};

DECLARE_DELEGATE_OneParam(FActivityStepResultDelegate, const FActivityStepResult&);

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
	 * /!\ May be called before StartStep. TODO: Fix this (in ActivityExecutor)
	 */
	UFUNCTION(BlueprintNativeEvent)
	void CancelStep();
	
protected:

	/**
	 * Notify the step executor that this step is finished.
	 * @param Output Result to report.
	 */
	UFUNCTION(BlueprintPure=false)
	void FinishStep(const FActivityStepResult& Output) const;
	
private:
	
	friend class UActivityExecutor;
	FActivityStepResultDelegate StepFinishedCallback;

protected: // Default implementations

	virtual void StartStep_Implementation(AActor* LastInstigator);
	virtual void OnInteract_Implementation(AActor* Instigator);
	virtual void CancelStep_Implementation();
};
