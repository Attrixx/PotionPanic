// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionBase.h"
#include "IFTInteraction.generated.h"

UENUM()
enum class EIFTStatus : uint8
{
	Default,
	WaitingForWindow,
	DuringWindow,
};

/**
 * 
 */
UCLASS()
class INTERACTIONS_API UIFTInteraction : public UInteractionBase
{
	GENERATED_BODY()
	
public:	
	void Init(UInteractionSetting* Settings) override;
	void StartInteraction(const FInteractionContext& Context) override;
	void InteractWhileProcess() override;
	
private:
	float SecondsBeforeWindow = 0.f;
	float WindowLengthSeconds = 1.f;
	
	FInteractionContext InteractionContext;
	
	FTimerHandle WindowHandle;
	EIFTStatus Status = EIFTStatus::Default;
	FInteractionOutput InteractionOutput;
};
