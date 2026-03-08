// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ActivityStep.generated.h"

UENUM(BlueprintType)
enum class EActivityResult : uint8
{
	Default,
	// CriticalSuccess, // TODO Maybe ?
	Success,
	Fail,
	CriticalFail
};

USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityOutput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EActivityResult ActivityResult = EActivityResult::Default;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;
};

DECLARE_DELEGATE_OneParam(FActivityOutputDelegate, const FActivityOutput&);

UCLASS(Abstract, Blueprintable)
class ACTIVITIES_API UActivityStep : public UObject
{
	GENERATED_BODY()

public:
	
	FActivityOutputDelegate OnActivityFinished;

	UFUNCTION(BlueprintNativeEvent)
	void StartActivity(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent)
	void InteractWhileProcess();
	
protected:
	
	UFUNCTION(BlueprintPure=false)
	void FinishActivity(const FActivityOutput& Output) const { OnActivityFinished.ExecuteIfBound(Output); }

protected:

	virtual void StartActivity_Implementation(AActor* Instigator) PURE_VIRTUAL(UActivityStep::RequiresPlayerInteraction,);
	virtual void InteractWhileProcess_Implementation() { }
};
