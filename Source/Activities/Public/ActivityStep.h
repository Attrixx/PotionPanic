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

DECLARE_DELEGATE_OneParam(FActivityOutputDelegate, const FActivityStepResult&);

UCLASS(Abstract, EditInlineNew, Blueprintable)
class ACTIVITIES_API UActivityStep : public UObject
{
	GENERATED_BODY()

public:
	
	FActivityOutputDelegate ActivityFinishedCallback;
	
	UFUNCTION(BlueprintNativeEvent)
	void StartStep(AActor* LastInstigator);

	UFUNCTION(BlueprintNativeEvent)
	void OnInteract(AActor* Instigator);
	
	UFUNCTION(BlueprintNativeEvent)
	void CancelStep();
	
protected:
	
	UFUNCTION(BlueprintPure=false)
	void FinishStep(const FActivityStepResult& Output) const;

protected: // Default implementations

	virtual void StartStep_Implementation(AActor* LastInstigator);
	virtual void OnInteract_Implementation(AActor* Instigator);
	virtual void CancelStep_Implementation();
};
