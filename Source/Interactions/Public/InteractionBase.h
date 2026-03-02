// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"
#include "InteractionBase.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	QTE UMETA(DisplayName = "QTE"),
	IFT UMETA(DisplayName = "IFT")
};

UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	None UMETA(DisplayName = "None"),
	Success UMETA(DisplayName = "Success"),
	Fail UMETA(DisplayName = "Fail"),
	Timeout UMETA(DisplayName = "Timeout"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionScoringRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	int32 BaseScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	int32 SuccessBonus = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	int32 FailurePenalty = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	int32 PerfectBonus = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	float PerfectTimeThresholdSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
	int32 MinScore = 0;
};

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "1"))
	int32 RequiredSuccessCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0"))
	int32 AllowedFailureCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0.1"))
	float MaxDurationSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	int32 PointsPerSuccess = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
	int32 PointsPerFailure = 25;
};

UCLASS(BlueprintType)
class INTERACTIONS_API UInteractionDefinitionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	EInteractionType Type = EInteractionType::QTE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	FInteractionScoringRule Scoring;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
	FInteractionRules Rules;
};

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionResult Result = EInteractionResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 SuccessCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 FailureCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 AttemptCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	float ElapsedSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionResult InteractionResult = EInteractionResult::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 SuccessCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 FailureCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float DurationSeconds = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionOutputDelegate, FInteractionOutput, InteractionOutput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionStateChangedDelegate, const FInteractionRuntimeState &, RuntimeState);

UCLASS(BlueprintType, Blueprintable)
class INTERACTIONS_API UInteractionBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool StartInteraction(const UInteractionDefinitionAsset *InDefinition);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterAttempt(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void AdvanceTime(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelInteraction();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	const FInteractionRuntimeState &GetRuntimeState() const { return RuntimeState; }

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FInteractionOutputDelegate OnInteractionFinished;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FInteractionStateChangedDelegate OnInteractionStateChanged;

protected:
	void FinishInteraction(EInteractionResult Result);
	int32 ComputeScore() const;

	int32 GetRequiredSuccessCount() const;
	int32 GetAllowedFailureCount() const;
	float GetMaxDurationSeconds() const;
	int32 GetPointsPerSuccess() const;
	int32 GetPointsPerFailure() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<const UInteractionDefinitionAsset> Definition = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	FInteractionRuntimeState RuntimeState;

	bool bIsRunning = false;
};
