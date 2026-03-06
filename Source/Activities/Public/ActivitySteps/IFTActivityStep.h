// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "IFTActivityStep.generated.h"

UCLASS()
class ACTIVITIES_API UIFTActivitySetting : public UActivityStepSettings
{
	GENERATED_BODY()

	UActivityStep* CreateStep(UObject* Outer) const override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	float SecondsBeforeWindow = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="")
	float WindowLengthSeconds = 1.f;
};

UENUM()
enum class EIFTStatus : uint8
{
	None = 0,
	WaitingForWindow,
	DuringWindow,
	PastWindow
};

/**
 * 
 */
UCLASS()
class ACTIVITIES_API UIFTActivityStep : public UActivityStep
{
	GENERATED_BODY()

public:

	void StartActivity(const FActivityContext& Context) override;
	void InteractWhileProcess() override;
	bool RequiresPlayerInteraction() const override { return false; }

private:

	friend UIFTActivitySetting;

	float SecondsBeforeWindow = 0.f;
	float WindowLengthSeconds = 1.f;

	FActivityContext ActivityContext;

	FTimerHandle WindowHandle;
	EIFTStatus Status = EIFTStatus::None;
	FActivityOutput ActivityOutput;
};
