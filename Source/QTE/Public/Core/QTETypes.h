#pragma once

#include "CoreMinimal.h"
#include "QTETypes.generated.h"

class UInputAction;
class UQTEDefinitionDataAsset;
class UTexture2D;
class AActor;

UENUM(BlueprintType)
enum class EQTEType : uint8
{
	Press,
	Hold,
	Mash,
	Sequence,
	Chain
};

UENUM(BlueprintType)
enum class EQTEStepType : uint8
{
	Press,
	Hold,
	Mash
};

UENUM(BlueprintType)
enum class EQTEState : uint8
{
	None,
	Running,
	Success,
	Failure,
	Timeout,
	Canceled,
	Interrupted
};

UENUM(BlueprintType)
enum class EQTEGrade : uint8
{
	None,
	Perfect,
	Good,
	Fail
};

USTRUCT(BlueprintType)
struct QTE_API FQTEPresentationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEStepFeedbackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText SuccessText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText FailureText;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEOutcomeConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FText Message;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	float GlobalTimeoutSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	float InputToleranceSeconds = 0.1f;

	/**
	 * Mistakes tolerated before the QTE fails. At 0 the first mistake already ends it, which is
	 * what bFailOnAnyMistake does -- so leave that one off and drive the tolerance from here.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	int32 MaxMistakes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	bool bCountUnexpectedPressedInputAsMistake = true;

	/**
	 * Fails on the first mistake whatever MaxMistakes says. Redundant at MaxMistakes = 0, and
	 * above it silently throws the allowance away -- prefer setting MaxMistakes alone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	bool bFailOnAnyMistake = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0.1))
	float DifficultyMultiplier = 1.f;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEStepDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	EQTEStepType StepType = EQTEStepType::Press;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FQTEPresentationData Presentation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	FQTEStepFeedbackData Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	bool bUseStepTimeout = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	float StepTimeoutSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	float RequiredHoldTimeSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 1))
	int32 RequiredMashCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE")
	bool bOverrideTolerance = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE", meta = (ClampMin = 0))
	float ToleranceOverrideSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct QTE_API FQTERuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UObject> SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UQTEDefinitionDataAsset> Definition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEType QTEType = EQTEType::Press;

	/**
	 * InputAction of the step being played, so a widget can show its key straight from the state
	 * it is already handed, instead of reaching back into the component for live data that may
	 * have been cleared by the time the broadcast lands. Null when no step is current.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UInputAction> CurrentInputAction = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEState Status = EQTEState::None;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 CurrentStepIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 CompletedStepCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 TotalSteps = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 Mistakes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 CurrentMashCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 EffectiveMashTarget = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float StepElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float GlobalTimeRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float StepTimeRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float EffectiveHoldTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float EffectiveStepTimeout = 0.f;

	// Denominator for GlobalTimeRemaining, so UI can show a normalised timer gauge.
	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float EffectiveGlobalTimeout = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float EffectiveTolerance = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float StepProgress = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float OverallProgress = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	bool bInputHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	FText FeedbackText;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UQTEDefinitionDataAsset> Definition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<UObject> SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEType QTEType = EQTEType::Press;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEState Outcome = EQTEState::None;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEGrade Grade = EQTEGrade::None;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 CompletedStepCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 FailedStepIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 Mistakes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	FText Message;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	FQTERuntimeState FinalRuntimeState;
};

USTRUCT(BlueprintType)
struct QTE_API FQTEAuthorityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEState Outcome = EQTEState::None;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	EQTEGrade Grade = EQTEGrade::None;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 CompletedStepCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 FailedStepIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	int32 Mistakes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	FText Message;

};
