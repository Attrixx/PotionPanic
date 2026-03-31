// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "IFTActivityStep.generated.h"

UCLASS(DisplayName = "IFT")
class ACTIVITIES_API UIFTActivitySetting : public UActivityStepSettings
{
	GENERATED_BODY()
	
protected:

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

	UPROPERTY(EditAnywhere, Category="")
	float SecondsBeforeWindow = 0.f;

	UPROPERTY(EditAnywhere, Category="")
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

	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;
	void CancelStep_Implementation() override;

	friend UIFTActivitySetting;

	float SecondsBeforeWindow = 0.f;
	float WindowLengthSeconds = 1.f;

	FTimerHandle WindowHandle;
	EIFTStatus Status = EIFTStatus::None;
};
