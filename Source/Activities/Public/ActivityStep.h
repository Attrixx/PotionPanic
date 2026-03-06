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

	EActivityResult ActivityResult = EActivityResult::Default;
	uint32 Score = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActivityOutputDelegate, const FActivityOutput&, ActivityOutput);

USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* Instigator;
	FActivityOutputDelegate OnActivityFinished;
};

class UActivityStepSettings;

UCLASS(Abstract, BlueprintType)
class ACTIVITIES_API UActivityStep : public UObject
{
	GENERATED_BODY()
	
public:	
	
	UFUNCTION(BlueprintCallable)
	virtual void StartActivity(const FActivityContext& Context) PURE_VIRTUAL(UActivityBase::StartActivity, );
	
	virtual void InteractWhileProcess() {}
	
	virtual bool RequiresPlayerInteraction() const PURE_VIRTUAL(UActivityStepSettings::RequiresPlayerActivity, return false;);
};

