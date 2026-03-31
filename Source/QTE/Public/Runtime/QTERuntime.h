#pragma once

#include "CoreMinimal.h"
#include "Core/QTETypes.h"

class UInputAction;
class UQTEComponent;
class UQTEDefinitionDataAsset;

class QTE_API FQTERuntime
{
public:
	static TUniquePtr<FQTERuntime> Create(UQTEComponent& OwnerComponent, UQTEDefinitionDataAsset& Definition);

	FQTERuntime(UQTEComponent& InOwnerComponent, UQTEDefinitionDataAsset& InDefinition);
	virtual ~FQTERuntime();

	virtual void Start() = 0;
	virtual void RefreshRuntimeState();
	bool TryAutoCompleteHoldStep();
	virtual bool HandlePressed(const UInputAction* InputAction) = 0;
	virtual bool HandleReleased(const UInputAction* InputAction) = 0;
	virtual bool HandleTriggered(const UInputAction* InputAction) = 0;

protected:
	void BeginCurrentStep(bool bBroadcastStepChanged);
	bool AdvanceToNextStep();
	const FQTEStepDefinition* ResolveInputStep(const UInputAction* InputAction, bool bTreatUnexpectedPressedAsMistake);
	void RefreshCurrentStepByType(const FQTEStepDefinition& Step);
	bool HandleCurrentStepPressedByType(const FQTEStepDefinition& Step);
	bool HandleCurrentStepReleasedByType(const FQTEStepDefinition& Step);
	bool HandleCurrentStepTriggeredByType(const FQTEStepDefinition& Step);
	void RefreshPressStepProgress();
	bool CompletePressStep();
	void RefreshHoldStepProgress();
	bool StartHoldStep();
	bool ReleaseHoldStep(const FQTEStepDefinition& Step);
	bool UpdateHoldStep();
	void RefreshMashStepProgress();
	bool AdvanceMashStep();

private:
	virtual void RefreshStepProgress(const FQTEStepDefinition& Step) = 0;
	const FQTEStepDefinition* GetCurrentStep() const;
	void HandleMistake(const FText& FailureText);
	FQTERuntimeState& GetMutableRuntimeState();
	const FQTERuntimeState& GetRuntimeState() const;
	float& GetHoldStartTime();
	void BroadcastUpdated();

private:
	UQTEComponent& OwnerComponent;
	UQTEDefinitionDataAsset& Definition;
};
