#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/QTERuntimeSettings.h"
#include "Core/QTETypes.h"
#include "QTEDefinitionDataAsset.generated.h"

class FDataValidationContext;
enum class EDataValidationResult : uint8;

UCLASS(BlueprintType)
class QTE_API UQTEDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	void PostLoad() override;
	void EnsureRuntimeSettings();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Definition")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "QTE|Definition")
	TObjectPtr<UQTERuntimeSettings> RuntimeSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Presentation")
	FQTEPresentationData Presentation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Rules")
	FQTEConfiguration Configuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Outcomes")
	FQTEOutcomeConfiguration SuccessOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Outcomes")
	FQTEOutcomeConfiguration FailureOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Outcomes")
	FQTEOutcomeConfiguration TimeoutOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Outcomes")
	FQTEOutcomeConfiguration CanceledOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Outcomes")
	FQTEOutcomeConfiguration InterruptedOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Steps")
	TArray<FQTEStepDefinition> Steps;

	UFUNCTION(BlueprintPure, Category = "QTE")
	int32 GetStepCount() const { return Steps.Num(); }

	UFUNCTION(BlueprintPure, Category = "QTE")
	FText GetResolvedDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	FText GetResolvedStepLabel(int32 StepIndex) const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	EQTEType GetQTEType() const;

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use RuntimeSettings."))
	EQTEType QTEType = EQTEType::Press;

public:

	float GetDifficultyMultiplier();
	float GetEffectiveGlobalTimeout();
	float GetEffectiveStepTimeout(const FQTEStepDefinition& Step);
	float GetEffectiveTolerance(const FQTEStepDefinition& Step);
	float GetEffectiveHoldTime(const FQTEStepDefinition& Step);
	int32 GetEffectiveMashTarget(const FQTEStepDefinition& Step);
	const FQTEOutcomeConfiguration* GetOutcomeConfiguration(EQTEState FinalState);
	EQTEGrade ResolveGrade(const FQTERuntimeState& RuntimeState, EQTEState FinalState);
	FText ResolveOutcomeMessage(EQTEState FinalState, const FText& OverrideMessage);
	FQTEResult BuildResult(
		UObject* SourceObject,
		AActor* InstigatorActor,
		const FQTERuntimeState& RuntimeState,
		EQTEState FinalState,
		const FText& OverrideMessage);
	FQTEAuthorityResult BuildAuthorityResult(
		const FQTERuntimeState& RuntimeState,
		const FQTEResult& LastResult,
		EQTEState FallbackState);
};
